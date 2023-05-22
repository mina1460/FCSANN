import math
from queue import PriorityQueue
import numpy as np

def ComputeL2Distance(pX, pY, data_dim):
    l2_dist = 0
    for i in range(data_dim):
        l2_dist += (pX[i] - pY[i])**2
    return l2_dist

def computeManhattanDistance(pX, pY, dimension):

    v1 = np.array(pX)
    v2 = np.array(pY)

    manhattan_dist = np.linalg.norm(v1 - v2, ord=1)
    return manhattan_dist


def ComputeFiltersDistance(pX_f, pY_f, dimension_f, filtersNum, pX, pY, dimension):
    W = 0.25
    filtersDistance = 0

    for x in range(filtersNum):
        i = 0
        while i < dimension_f:
            if pX_f[i] != pY_f[i]:
                break
            i += 1

        if i == dimension_f:
            continue
        else:
            dist = ComputeL2Distance(pX, pY, dimension)
            bias = 10 * (W * dist + 3.322)
            expo = computeManhattanDistance(pX_f[x:], pY_f[x:], dimension_f)
            logg = math.log10(expo + 2)
            # print("bias: ", bias, " expo: ", expo, " logg: ", logg)
            filtersDistance += bias - (1 / logg)

    return filtersDistance / filtersNum


def computeFusionDistance(l2_dist, fil_dist):
    W = 0.25
    return W * l2_dist + fil_dist

def printQueue(queue):
    while not queue.empty():
        dist, index = queue.get()
        print("Distance: ", dist, " Index: ", index)

def main():
    data_dim = 5
    filter_dim = 2
    filters_num = 3
    data_num = 1
    # data = [[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12], [13, 14, 15], [16, 17, 18], [19, 20, 21], [22, 23, 24], [25, 26, 27], [28, 29, 30]]
    # data_filters = [[[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[10], [9], [1], [2], [8]], [[10], [9], [1], [2], [8]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]]]
    # query_point = [15, 16, 17]
    # query_point_filter = [[1], [3], [8], [9], [6]]
    data = [[56,85,19,13,49]]
    data_filters = [[[0,3], [4,3], [5,3]]]
    query_point = [9,15,53,20,63]
    query_point_filter = [[1,3], [4,3], [5,0]]

    rank = PriorityQueue()
    for i in range(data_num):
        fil_dist = ComputeFiltersDistance(query_point_filter, data_filters[i], filter_dim, filters_num, query_point, data[i], data_dim)
        l2_dist = ComputeL2Distance(query_point, data[i], data_dim)
        fusion_dist = computeFusionDistance(l2_dist, fil_dist)
        rank.put((fusion_dist, i))
        print("Fusion Distance: ", fusion_dist, " L2 Distance: ", l2_dist, " Filters Distance: ", fil_dist)
        

    print("The queue is:")
    printQueue(rank)

if __name__ == '__main__':
    main()
