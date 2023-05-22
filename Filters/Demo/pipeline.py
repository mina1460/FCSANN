from sentence_transformers import SentenceTransformer
from text_model import Result
import pandas as pd
import numpy as np
from data import Data
from ordering import order_filters
from heaps import MaxHeap

import Best

try:
    import SPTAG
except:
    print("SPTAG not installed")

# star rating
STARS = ["1", "2", "3", "4", "5"]
# verified purchase
VERIFIED = ["0", "1"]
# years until 2023
YEARS = [str(x) for x in range(0, 17)]
# all categories
CAT_NUM = [str(x) for x in range(0, 8)]
CATEGORIES = [
    "Baby",
    "Camera",
    "Digital Music",
    "Jewelry",
    "Musical Instruments",
    "Personal Care Appliances",
    "Tools",
    "Video Games",
    "Watches",
]


class TextPipeline:
    def __init__(self):
        self.df = pd.read_csv("data/full_train_data.csv")
        self.min_year = 1999
        self.data = Data()
        self.model = SentenceTransformer("sentence-transformers/all-MiniLM-L12-v1")
        self._init_index(SPTAG)

    def _init_index(self, library, mode=1):
        print("Initializing Index...")

        num_elements = self.data.train.shape[0]
        dim = self.data.train.shape[1]

        filter_num = self.data.filters_train.shape[1]
        filter_dim = self.data.filters_train.shape[2]

        self.index = library.AnnIndex("SPANN", "Float", dim)
        self.index.SetBuildParam("DistCalcMethod", "L2", "Base")
        self.index.SetBuildParam("IndexAlgoType", "BKT", "Base")
        self.index.SetBuildParam("isExecute", "true", "SelectHead")
        self.index.SetBuildParam("Ratio", "0.16", "SelectHead")
        self.index.SetBuildParam("NumberOfThreads", "64", "SelectHead")
        self.index.SetBuildParam("isExecute", "true", "BuildHead")
        self.index.SetBuildParam("RefineIterations", "3", "BuildHead")
        self.index.SetBuildParam("NumberOfThreads", "64", "BuildHead")
        self.index.SetBuildParam("isExecute", "true", "BuildSSDIndex")
        self.index.SetBuildParam("BuildSsdIndex", "true", "BuildSSDIndex")
        self.index.SetBuildParam("InternalResultNum", "64", "BuildSSDIndex")
        self.index.SetBuildParam("PostingPageLimit", "12", "BuildSSDIndex")
        self.index.SetBuildParam("NumberOfThreads", "64", "BuildSSDIndex")
        self.index.SetBuildParam("SearchInternalResultNum", "200", "BuildSSDIndex")
        self.index.SetBuildParam("SearchPostingPageLimit", "12", "BuildSSDIndex")
        self.index.SetBuildParam("isExecute", "true", "SearchSSDIndex")
        self.index.SetBuildParam("BuildSsdIndex", "false", "SearchSSDIndex")

        flattened_train = self.data.train.flatten().astype(np.float32)
        flattened_filter = self.data.filters_train.flatten().astype(np.float32)

        self.index.BuildWithFilters(
            flattened_train, flattened_filter, num_elements, filter_num, filter_dim
        )

    def query(self, data, filters, k):
        # Stage 1 - Embedding
        query_embedding = self._embed_sentences(data)
        # query_embedding = data

        filters_embedding = self.all_pos(filters)

        k_orig = int(k)
        k_more = k_orig * 5

        maxHeap = MaxHeap()

        # Stage 2 - Find KNN using SPTAG with filtered search
        unique_indices = set()
        for filter in filters_embedding:
            indices, distances = self._find_knn(query_embedding, filter, k_more)

            for i in range(len(indices)):
                if indices[i] not in unique_indices:
                    unique_indices.add(indices[i])
                    if len(maxHeap) >= k_more:
                        if distances[i] < maxHeap[0][0]:
                            maxHeap.heappop()
                            maxHeap.heappush((distances[i], indices[i]))
                    else:
                        maxHeap.heappush((distances[i], indices[i]))

        # import pdb

        # pdb.set_trace()

        heaps = np.array(maxHeap.get_heap())
        top_indices = heaps[:, 1].astype(int)
        top_distances = heaps[:, 0]

        try:
            if len(filters_embedding) == 1:
                indices_ordered, distances_ordered = order_filters(
                    k_orig,
                    top_indices,
                    top_distances,
                    filters_embedding[0],
                    "data/train_filters.csv",
                )
            else:
                filters_to_order_by = []
                for f in filters:
                    if len(f) > 1:
                        filters_to_order_by.append(-1)
                    else:
                        filters_to_order_by.append(f[0])
                indices_ordered, distances_ordered = order_filters(
                    k_orig,
                    top_indices,
                    top_distances,
                    filters_to_order_by,
                    "data/train_filters.csv",
                )
        except:
            print("Except")
            indices_ordered = top_indices
            distances_ordered = top_distances

        # Stage 3 - Reverse mapping
        result = self._reverse_map(indices_ordered, distances_ordered)

        return result

    def all_pos(self, filters):
        filter_all_pos = []

        for rating in filters[0]:
            for vpurchase in filters[1]:
                for year in filters[2]:
                    for category in filters[3]:
                        filter_all_pos.append([rating, vpurchase, year, category])

        return filter_all_pos

    def _embed_sentences(self, sentences):
        print("Embedding sentences...")
        embeddings = self.model.encode(sentences)
        return embeddings

    def _embed_category(self, category):
        print("Embedding filters...")
        return self.category_map[category]

    def _embed_year(self, year):
        print("Embedding filters...")
        return int(year) - self.min_year

    def _find_knn(self, query, filters, k):
        print("Finding KNN...")
        filtersNum = self.data.filters_train.shape[1]
        filtersDim = self.data.filters_train.shape[2]

        # max_len = len(max(filters, key=len))
        # print(max_len)
        flattened_test = query.flatten().astype(np.float32)

        # for i in range(max_len):

        # filter = [filters[0][i], filters[1][i], filters[2][i], filters[3][i]]
        flattened_filters = np.array(filters).flatten().astype(np.float32)
        temp = self.index.SearchWithFilters(
            flattened_test, flattened_filters, filtersNum, filtersDim, k
        )

        return np.array(temp[0]), np.array(temp[1])

        indices = []
        for i in range(k):
            indices.append(i)

        distances = [0] * k

        return indices, distances

    def _reverse_map(self, indices, distances):
        print("Reverse mapping...")
        result = []
        for i, index in enumerate(indices):
            row = self.df.iloc[index]
            text = row["review_body"]
            f1 = row["star_rating"]
            f2 = row["verified_purchase"]
            f3 = self.get_year(row["year"])
            f4 = row["product_category"]
            id = row["product_id"]
            product = row["product_title"]
            result.append(
                Result(index, distances[i], text, f1, f2, f3, f4, id, product)
            )

        return result

    def get_category(self, category):
        return CATEGORIES[int(category)]

    def get_year(self, year):
        return year + self.min_year

    def get_filter(self, filter, value):
        if value != "-1":
            if filter == "year":
                return [self._embed_year(value)]
            return [value]
        else:
            if filter == "rating":
                return STARS
            elif filter == "vpurchase":
                return VERIFIED
            elif filter == "year":
                return YEARS
            elif filter == "category":
                return CAT_NUM
