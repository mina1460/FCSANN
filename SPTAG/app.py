#!/opt/homebrew/bin/python3.8
from dataclasses import dataclass
import logging
import numpy as np
import h5py
import click
import typing as t
import os
import time
import SPTAG
import shutil
import pandas as pd
from datetime import datetime
from flask import Flask, render_template, request
import wtforms
from wtforms import Form, StringField, TextAreaField
from wtforms.validators import InputRequired, Length
import pymongo
from pymongo import MongoClient
from flask_cors import CORS, cross_origin


from sentence_transformers import SentenceTransformer

embedding_model = SentenceTransformer("sentence-transformers/msmarco-MiniLM-L6-cos-v5")
df = pd.read_csv("../datasets/finalMerged.csv")


# logging.basicConfig(level=logging.INFO, filename='./logs/ANN.log', format='%(asctime)s - {%(levelname)s} - %(message)s')

datasets = [
    "lastfm-64-dot",
    "fashion-mnist-784-euclidean",
    "gist-960-euclidean",
    "mnist-784-euclidean",
    "sift-128-euclidean",
]


@dataclass
class Data:
    """a representation of the data loaded from the hdf5 file"""

    distances: np.ndarray
    neighbors: np.ndarray
    train: np.ndarray
    test: np.ndarray
    distance: str

    def stats(self) -> None:
        """return a string with some stats about the data"""
        click.echo("\n_______________________Stats_______________________\n")
        click.echo(
            click.style("Train:", bold=True)
            + f" {self.train.shape}"
            + f" -- This is the distances between all {self.train.shape[0]} vertices"
        )
        click.echo(
            click.style("Distances:", bold=True)
            + f" {self.distances.shape}"
            + f" -- This is the distance to the nearest {self.distances.shape[1]} neighbors"
        )
        click.echo(
            click.style("Neighbors:", bold=True)
            + f" {self.neighbors.shape}"
            + f" -- This is the indices of the nearest {self.distances.shape[1]} neighbors"
        )

        click.echo(
            click.style("Test:", bold=True)
            + f" {self.test.shape}"
            + " -- This is the test data that we have the ground truth for"
        )
        click.echo(click.style("Distance:", bold=True) + f" {self.distance}")
        click.echo("\n___________________________________________________\n")


class ANN:
    """The main driver of everything, loads data, builds graphs, runs queries"""

    def __init__(
        self, filepath, n_neighbors: int, batch: bool, in_memory: bool
    ) -> None:
        self.filepath = filepath
        self.n_neighbors = n_neighbors
        self.batch = batch
        self.in_memory = in_memory
        self.file: h5py.File
        self.data: Data

    def load_data(self, verbose=False) -> None:
        """loads the data from the hdf5 file either in memory or out of memory"""
        driver = None
        if self.in_memory:
            driver = "core"
        file = h5py.File(self.filepath, "r", driver=driver)
        self.data = Data(
            distances=np.array(file.get("distances")),
            neighbors=np.array(file.get("neighbors")),
            train=np.array(file.get("train")),
            test=np.array(file.get("test")),
            distance=file.attrs["distance"],
        )

        click.secho(f"Loaded data from {self.filepath}", fg="green")
        if verbose:
            self.data.stats()
            click.secho("\nDistances: ", fg="green")
            print(self.data.distances)
            click.secho("\nNeighbors: ", fg="green")
            print(self.data.neighbors)
            click.secho("\nTrain: ", fg="green")
            print(self.data.train)
            click.secho("\nTest: ", fg="green")
            print(self.data.test)

    def construct_graph(self, algorithm):
        """constructs the graph from the data"""
        click.echo(
            click.style(
                "BUILDING GRAPH using " + algorithm + " ...", blink=True, bold=True
            )
        )
        num_elements = self.data.train.shape[0]
        dim = self.data.train.shape[1]
        click.secho(f"{num_elements} elements, {dim} dimensions", fg="blue")
        build_time_start = time.time()

        filename = self.filepath.split("/")[-1].split("-")[0]
        if algorithm == "spann":
            if os.path.exists("sptag_index_spann_{}".format(filename)):
                self.index = SPTAG.AnnIndex.Load(
                    "sptag_index_spann_{}".format(filename)
                )
            else:
                print("Begin SPANN")

                self.index = SPTAG.AnnIndex("SPANN", "Float", dim)
                # self.index.SetBuildParam("DistCalcMethod", 'L2', "Index")
                # self.index.SetBuildParam("NumberOfThreads", '4', "Index")

                # [Base]
                # self.index.SetBuildParam("ValueType", 'Float', "Base")
                self.index.SetBuildParam("DistCalcMethod", "L2", "Base")
                self.index.SetBuildParam("IndexAlgoType", "BKT", "Base")
                # self.index.SetBuildParam("Dim", f"{dim}", "Base")
                # self.index.SetBuildParam("NumberOfThreads", '4', "Base")

                # [SelectHead]
                # selecting the number of centroids depenfing on "ratio" of original data or "count"
                self.index.SetBuildParam("isExecute", "true", "SelectHead")
                # self.index.SetBuildParam("TreeNumber", '1', "SelectHead")
                # self.index.SetBuildParam("BKTKmeansK", '32', "SelectHead")
                # self.index.SetBuildParam("BKTLeafSize", '8', "SelectHead")
                # self.index.SetBuildParam("SamplesNumber", '1000', "SelectHead")
                # self.index.SetBuildParam("SaveBKT", 'false', "SelectHead")
                # self.index.SetBuildParam("SelectThreshold", '50', "SelectHead")
                # self.index.SetBuildParam("SplitFactor", '6', "SelectHead")
                # self.index.SetBuildParam("SplitThreshold", '100', "SelectHead")
                self.index.SetBuildParam("Ratio", "0.16", "SelectHead")
                self.index.SetBuildParam("NumberOfThreads", "64", "SelectHead")
                # self.index.SetBuildParam("BKTLambdaFactor", '-1', "SelectHead")

                # [BuildHead]
                self.index.SetBuildParam("isExecute", "true", "BuildHead")
                # self.index.SetBuildParam("NeighborhoodSize", '32', "BuildHead")
                # self.index.SetBuildParam("TPTNumber", '32', "BuildHead")
                # self.index.SetBuildParam("TPTLeafSize", '2000', "BuildHead")
                # self.index.SetBuildParam("MaxCheck", '8192', "BuildHead")
                # self.index.SetBuildParam("MaxCheckForRefineGraph", '8192', "BuildHead")
                self.index.SetBuildParam("RefineIterations", "3", "BuildHead")
                self.index.SetBuildParam("NumberOfThreads", "64", "BuildHead")
                # self.index.SetBuildParam("BKTLambdaFactor", '-1', "BuildHead")

                # [BuildSSDIndex]
                self.index.SetBuildParam("isExecute", "true", "BuildSSDIndex")
                self.index.SetBuildParam("BuildSsdIndex", "true", "BuildSSDIndex")
                self.index.SetBuildParam("InternalResultNum", "64", "BuildSSDIndex")
                # self.index.SetBuildParam("ReplicaCount", '8', "BuildSSDIndex")
                self.index.SetBuildParam("PostingPageLimit", "12", "BuildSSDIndex")
                self.index.SetBuildParam("NumberOfThreads", "64", "BuildSSDIndex")
                # self.index.SetBuildParam("MaxCheck", '8192', "BuildSSDIndex")
                self.index.SetBuildParam(
                    "SearchInternalResultNum", "64", "BuildSSDIndex"
                )
                self.index.SetBuildParam(
                    "SearchPostingPageLimit", "12", "BuildSSDIndex"
                )
                # self.index.SetBuildParam("SearchResult", 'result.txt', "BuildSSDIndex")
                # self.index.SetBuildParam("ResultNum", '10', "BuildSSDIndex")
                # self.index.SetBuildParam("MaxDistRatio", '8.0', "BuildSSDIndex")

                # [SearchSSDIndex]
                self.index.SetBuildParam("isExecute", "true", "SearchSSDIndex")
                self.index.SetBuildParam("BuildSsdIndex", "false", "SearchSSDIndex")
                # self.index.SetBuildParam("InternalResultNum", '32', "SearchSSDIndex")
                # self.index.SetBuildParam("NumberOfThreads", '1', "SearchSSDIndex")
                # self.index.SetBuildParam("HashTableExponent", '4', "SearchSSDIndex")
                # self.index.SetBuildParam("ResultNum", '10', "SearchSSDIndex")
                # self.index.SetBuildParam("MaxCheck", '2048', "SearchSSDIndex")
                # self.index.SetBuildParam("MaxDistRatio", '8.0', "SearchSSDIndex")
                # self.index.SetBuildParam("SearchPostingPageLimit", '12', "SearchSSDIndex")

                if os.path.exists("sptag_index_spann_{}".format(filename)):
                    shutil.rmtree("sptag_index_spann_{}".format(filename))
                if self.index.Build(self.data.train, num_elements, False):
                    self.index.Save(
                        "sptag_index_spann_{}".format(filename)
                    )  # Save the index to the disk

        build_time_end = time.time()
        click.secho(
            f"Build time: {(build_time_end - build_time_start):.3f} seconds\n",
            fg="bright_blue",
        )

    def query(self, algorithm) -> np.ndarray:
        """queries the graph for the nearest neighbors"""

        print(type(self.data.test))
        print(self.data.test.shape)
        result = []
        no_of_q = 1
        # temp = self.data.test

        # To increase the number of queries
        # for _ in range(1):
        #     # rand_int = np.random.randint(0, 5, temp.shape)
        #     # t = np.add(temp, rand_int)
        #     self.data.test = np.concatenate((self.data.test, self.data.test), axis=0)
        #     self.data.neighbors = np.concatenate((self.data.neighbors, self.data.neighbors), axis=0)
        click.secho(f"Test shape: {self.data.test.shape}", fg="bright_blue", bold=True)
        ###########################

        # for i in self.data.test:
        #     result.append (self.index.Search(i, self.n_neighbors)[0])
        #     neighbors = self.index.Search(i, self.n_neighbors)[0]
        #     print(f"neighbors: {len(neighbors)}")
        # sp_result = np.array(result)

        # print(f"NEW Test shape: {self.data.test.shape}")

        ### DEBUGGING PURPOSES (Never test with it) ###
        # self.data.test = self.data.test
        # self.data.test = self.data.test[22:23]
        SPTAG_t1 = self.data.test
        FAKASULO_t1 = self.data.test
        ##########################

        query_time_start = time.time()
        # result = self.index.BatchSearch(self.data.test, self.data.test.shape[0], self.n_neighbors, False)
        # print data.test type
        print(type(self.data.test))
        print(self.data.test.shape)
        print(self.data.test.shape[0])

        # sp_result = self.index.BatchSearch(SPTAG_t1, self.data.test.shape[0], self.n_neighbors, False)
        # sp_result = np.array(sp_result[0]).reshape((self.data.test.shape[0], self.n_neighbors))

        # To normalize the vector => x / np.linalg.norm(x)
        # result = self.index.FakasuloSearchProdCons(FAKASULO_t1, self.data.test.shape[0], self.n_neighbors, False, 8)

        result = self.index.FakasuloSearch(
            FAKASULO_t1, self.data.test.shape[0], self.n_neighbors, False, 8
        )
        # print("result Shape: ", result.shape)
        # print first 5 results
        query_time_end = time.time()
        result = np.array(result[0]).reshape(
            (self.data.test.shape[0], self.n_neighbors)
        )
        print("result[0:5]: ", result[0:5])

        # print result
        # print(f"Result shape: {result.shape}")
        # print (f"Result: {result}")

        # print(f"SP Result shape: {sp_result.shape}")
        # print (f"SP Result: {sp_result}")

        ############DEBUGGINF PURPOSES############
        # sp_result = []
        # result    = []

        # limit = 10000
        # SPTAG_t1_arr = self.data.test[:limit]
        # sp_result = self.index.BatchSearch(SPTAG_t1_arr, limit, self.n_neighbors, False)

        # FAKASULO_t1_arr = self.data.test[:limit]
        # result = self.index.FakasuloSearch(FAKASULO_t1_arr, limit, self.n_neighbors, False, 1)

        # sp_result = np.array(sp_result[0]).reshape((limit, self.n_neighbors))
        # result = np.array(result[0]).reshape((limit, self.n_neighbors))

        # print(f"SP Result shape: {sp_result.shape}")
        # # print (f"SP Result: {sp_result}")
        # print(f"Result shape: {result.shape}")
        # # print (f"Result: {result}")

        # # Set the distance type. Currently SPTAG only support Cosine and L2 distances. Here Cosine distance is not the Cosine similarity. The smaller Cosine distance it is, the better.

        # break_flag = True
        # counter = 0

        # for i,q in enumerate(zip(sp_result, result)):
        #     for vid_sp, vid_fa in zip (q[0], q[1]):
        #         if (vid_sp != vid_fa):
        #             if break_flag:
        #                 print(f"i {i} sp: {vid_sp} fa: {vid_fa}")
        #                 print(f"SPTAG: {sp_result[i]}\n")
        #                 print(f"FAKASULO: {result[i]}\n")
        #                 break_flag = False
        #             counter += 1
        #             break

        # print(f"Counter: {counter}")
        # print(f"Result shape: {len(result[0])}")
        # print(f"Result new shape: {result.shape}")

        # print(f"First 10 results: {result[:10]}")
        ############DEBUGGINF PURPOSES############

        # print(f"First 10 sp_results: {sp_result[:10]}")
        # df= pd.DataFrame(result)
        # df.to_csv('results.csv')
        labels = result

        click.secho("nearest neighbors: ", fg="green")
        click.secho(
            f"Query time: {(query_time_end - query_time_start):.4f} seconds\n",
            fg="bright_blue",
        )
        click.secho(
            f"Query per second: {(self.data.test.shape[0]/(query_time_end - query_time_start)):.4f}",
            fg="red",
            bold=True,
        )
        print(type(self.data.neighbors[:, : self.n_neighbors]))
        return labels

    def search_single_query(self, query_vector) -> np.ndarray:
        """queries the graph for the nearest neighbors"""
        print(query_vector.shape)
        result = []
        no_of_q = 1
        # result = self.index.FakasuloSearchProdCons(query_vector, query_vector.shape[0], self.n_neighbors, False, 8)
        result = self.index.BatchSearch(
            query_vector, query_vector.shape[0], self.n_neighbors, False
        )
        indices = np.array(result[0]).reshape((query_vector.shape[0], self.n_neighbors))
        distances = np.array(result[1]).reshape(
            (query_vector.shape[0], self.n_neighbors)
        )

        return indices, distances

    def k_recall_at_k(self, labels, ground_truth) -> float:
        """calculating the k recall at k"""
        print(f"NEW Test shape: {ground_truth.shape}")

        return np.mean(
            [
                (len(set(x).intersection(set(y))) / labels.shape[1])
                for (x, y) in zip(labels, ground_truth)
            ]
        )

    def evaluate(self, labels) -> float:
        """evaluates the recall of the query"""
        click.secho("evaluating recall: ", fg="green")
        print(self.data.neighbors[:, : self.n_neighbors].shape)
        print(len(labels))
        labels = np.array([np.array(i) for i in labels])
        print(self.data.neighbors[100][0:10])
        k_recall_k = self.k_recall_at_k(
            labels, self.data.neighbors[:, : self.n_neighbors - 1]
        )
        click.secho(f"k-recall@k: {k_recall_k}", fg="red", bold=True)

        return 0


# A form class for the search form
class ReviewForm(Form):
    Query = StringField("", [InputRequired()], description="Input Your Query")


# Global variables to serve multiple functions
ann = ANN("/home/g6/Desktop/dataset/datasets/merged.hdf5", 10, False, True)


def retrieve_docs(query):
    # generate the embedding vectors
    print(f"query to embedd {query}")
    embedding = embedding_model.encode(query)

    # run spann to get the 10 nearest neighbors
    out = ann.search_single_query(embedding)
    indices = out[0].tolist()[0]
    distances = out[1].tolist()[0]

    # Reverse mapping the indices from the local database
    documents = df.iloc[indices]
    documents = documents.to_dict(orient="records")

    # print(documents)
    # return all the docs data as a list of dicts
    return documents, distances


def entry_point() -> None:
    # load the AG News dataset and construct the sptag index
    ann.load_data(verbose=False)

    ann.construct_graph("spann")


def create_app():
    # Initaite the flask app with running the graph construction function before that
    app = Flask(__name__)
    CORS(app)

    with app.app_context():
        entry_point()

    @app.route("/")
    def index():
        form = ReviewForm(request.form)
        return render_template("home.html", form=form)

    @app.route("/results", methods=["POST"])
    def results():
        form = ReviewForm(request.form)
        if request.method == "POST":
            print("Query Received: ")
            query = [x for x in request.form.values() if x != "Search"]
            documents, distances = retrieve_docs(query)
            return render_template(
                "results.html",
                content=query,
                prediction=(documents[:10], distances),
            )
        return render_template("home.html", form=form)

    @app.route("/search", methods=["POST"])
    def search():
        form = ReviewForm(request.form)
        data = request.get_json()
        print(request.method)
        print(f"query: {data['query']}")
        if request.method == "POST" and data["query"] != "":
            print("Query Received: ")
            # query = [x for x in request.form.values() if x != "Search"]
            documents, distances = retrieve_docs([data["query"]])
            response_dict = {"docs": documents, "dist": distances}
            return response_dict
        return render_template("home.html", form=form)

    @app.route("/article/<int:id>")
    def show_article(id):
        content = df.loc[df["Id"] == id, "document"].iloc[0]
        return render_template("article.html", content=content)

    @app.route("/thanks", methods=["POST"])
    def feedback():
        return render_template("thanks.html")

    return app


if __name__ == "__main__":
    app = create_app()
    app.run(host="127.0.0.2", port=5005)
