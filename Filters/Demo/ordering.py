import pandas as pd


def order_filters(k, input_indices, input_distances, query_filters, filters_filename):
    """
    k: number of neighbors to return
    input_indices: list of indices of nearest neighbors of the query
    input_distances: list of distances of nearest neighbors of the query
    query_filters: list of filters from query
    filters_filename: name of the file containing the train filters

    Returns: list of indices of the k nearest neighbors of the query, ordered by similarity to the query
    (filters then text) and list of their corresponding distances
    """

    query_filters = [int(f) for f in query_filters]

    # print(query_filters)

    sorted_indices = sorted(input_indices)
    sorted_dist = [x for _, x in sorted(zip(input_indices, input_distances))]

    # import pdb

    # pdb.set_trace()

    # add 1 to the indices to account for the header row
    indices_to_read = [i + 1 for i in sorted_indices]

    # reading the filters file
    input_filters = pd.read_csv(
        filters_filename, skiprows=lambda x: x not in indices_to_read and x != 0
    )

    # adding the indices and distances column
    input_filters["indices"] = sorted_indices
    input_filters["distances"] = sorted_dist

    # adding new column that contains the number of similar filters to query filters
    input_filters["num_similar_filters"] = input_filters.apply(
        lambda row: len(set(row[0:4].tolist()).intersection(query_filters)),
        axis=1,
    )

    # sorting dataframe based on num_similar_filters and distances
    input_filters = input_filters.sort_values(
        by=["num_similar_filters", "distances"], ascending=[False, True]
    )

    new_k = min(k, input_filters.shape[0] - 1)

    # getting the indices of the first k rows
    output_indices = input_filters["indices"].iloc[:new_k].tolist()
    output_distances = input_filters["distances"].iloc[:new_k].tolist()

    print(input_filters)
    # print(output_indices)
    # print(output_distances)

    return output_indices, output_distances


# order_filters(2, [2, 1, 6], [0.1, 0.2, 0.3], [5, 1, 14, 0], "data/train_filters.csv")
