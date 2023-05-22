from dataclasses import dataclass
import numpy as np


def read_data_from_file(file_name):
    """Read data from file"""
    with open(file_name, "r") as f:
        data = []
        for line in f:
            # skip the first line
            if line[0] != '"':
                continue

            line = line.strip()
            line = line[2:-2]
            data.append([float(x) for x in line.split(",")])

    return data


def read_filter_from_file(file_name):
    """Read filters from file"""
    with open(file_name, "r") as f:
        filters = []
        first_line = True
        for line in f:
            # skip the first line
            if first_line:
                first_line = False
                continue
            per_node = []
            line = line.strip()
            for x in line.split(","):
                per_node.append([float(x)])

            filters.append(per_node)

    return filters


def read_ground_truth(file_name):
    """Read ground truth from file"""
    with open(file_name, "r") as f:
        ground_truth = []
        for line in f:
            line = line.strip()
            ground_truth.append([float(x) for x in line.split(",")])

    return ground_truth


def read_indices(file_name):
    """Read ground truth from file"""
    with open(file_name, "r") as f:
        ground_truth = []
        for line in f:
            line = line.strip()
            ground_truth.append([int(x) for x in line.split(",")])

    return ground_truth


@dataclass
class Data:
    """a class to store data"""

    neighbors: np.ndarray
    train: np.ndarray
    test: np.ndarray
    filters_train: np.ndarray
    filters_test: np.ndarray
    distances: np.ndarray

    def __init__(self) -> None:
        self.real_data()

    def real_data(self) -> None:
        """load real data from file"""
        dir_path_orig = "/home/g6/Filters/FCSANN/Filters/Data/Data_100000"

        train_data = read_data_from_file(dir_path_orig + "/train_data.csv")
        # test_data = read_data_from_file(dir_path_orig + "/test_data.csv")
        train_filter = read_filter_from_file(dir_path_orig + "/train_filters.csv")
        # test_filter = read_filter_from_file(dir_path_orig + "/test_filters.csv")

        # ground_truth = read_indices(dir_path_orig + "/indices_FWeight3x3")
        # distances = read_ground_truth(dir_path_orig + "/distances_FWeight3x3")

        self.train = np.array(train_data)
        # self.test = np.array(test_data)
        self.filters_train = np.array(train_filter)
        # self.filters_test = np.array(test_filter)
        # self.neighbors = np.array(ground_truth, dtype=object)
        # self.distances = np.array(distances)
