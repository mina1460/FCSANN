from fusion_distance import *
import threading
import time

# For real data
def read_data_from_file (file_name):
    '''Read data from file'''
    with open(file_name, 'r') as f:
        data = []
        for line in f:
            # skip the first line
            if line[0] != '\"':
                continue

            line = line.strip()
            line = line[2:-2]
            data.append([float(x) for x in line.split(',')])

    return data

def read_filters_from_file (file_name):
    '''Read filters from file'''
    with open(file_name, 'r') as f:
        filters = []
        first_line = True
        for line in f:
            # skip the first line
            if first_line:
                first_line = False
                continue
            per_node = []
            line = line.strip()
            for x in line.split(','):
                per_node.append([float(x)])

            filters.append(per_node)

    return filters


def read_vector_from_file (file_name):
    '''Read vectors from file'''
    with open(file_name, 'r') as f:
        vectors = []
        for line in f:
            vector = [int(x) for x in line.split(',')]
            vectors.append(vector)
    return vectors

def read_filter_from_file (file_name):
    '''Read filters from file'''
    with open(file_name, 'r') as f:
        filters = []
        for line in f:
            filter = [x for x in line.split('],[')]
            filter[0] = filter[0][1:]
            filter[-1] = filter[-1][:-2]
            for i in range(len(filter)):
                filter[i] = [int(x) for x in filter[i].split(',')]

            filters.append(filter)

    return filters


def compute_distance (test_filter, train_filter, filter_dim, filter_num, test_vector, train_vector, data_dim):
    '''Compute distance between two vectors and two filters'''
    distance = 0
    # for i in range(len(vector1)):
    #     distance += (vector1[i] - vector2[i])**2

    fil_dist = ComputeFiltersDistance(test_filter, train_filter, filter_dim, filter_num, test_vector, train_vector, data_dim)
    l2_dist = ComputeL2Distance(test_vector, train_vector, data_dim)
    distance = computeFusionDistance(l2_dist, fil_dist)
    # print(fil_dist, l2_dist, distance)

    return distance, l2_dist


def compute_distance_thread(test_filter, train_filter, filter_dim, filter_num, test_vector, train_vector, data_dim, result, result_l2, i, j):
    distance, l2_dist = compute_distance(test_filter[i], train_filter[j], filter_dim, filter_num, test_vector, train_vector, data_dim)
    result[i].append([j, distance])
    result_l2[i].append([j, l2_dist])


def generate_ground_truth_threads (train_data, test_data, train_filter, test_filter):
    '''Generate ground truth'''

    data_dim = len(train_data[0])

    filter_num = len(train_filter[0])
    filter_dim = len(train_filter[0][0])

    print('data_dim: ', data_dim)
    print('filter_num: ', filter_num)
    print('filter_dim: ', filter_dim)

    ground_truth = []
    ground_truth_l2 = []

    threads = []
    for i, test_vector in enumerate(test_data):
        ground_truth.append([])
        ground_truth_l2.append([])
        for j, train_vector in enumerate(train_data):
            thread = threading.Thread(target=compute_distance_thread, args=(test_filter, train_filter, filter_dim, filter_num, test_vector, train_vector, data_dim, ground_truth, ground_truth_l2, i, j))
            threads.append(thread)
            thread.start()
        
        for thread in threads:
            thread.join()

        ground_truth[i].sort(key=lambda x: x[1])
        ground_truth_l2[i].sort(key=lambda x: x[1])

    return ground_truth, ground_truth_l2

def generate_ground_truth (train_data, test_data, train_filter, test_filter):
    '''Generate ground truth'''
    
    data_dim = len(train_data[0])

    filter_num = len(train_filter[0])
    filter_dim = len(train_filter[0][0])

    print('data_dim: ', data_dim)
    print('filter_num: ', filter_num)
    print('filter_dim: ', filter_dim)

    ground_truth = []
    ground_truth_l2 = []
    for i, test_vector in enumerate(test_data):
        ground_truth.append([])
        ground_truth_l2.append([])
        for j, train_vector in enumerate(train_data):
            distance, l2_dist = compute_distance(test_filter[i], 
                                        train_filter[j],
                                        filter_dim,
                                        filter_num, 
                                        test_vector, 
                                        train_vector, 
                                        data_dim)
            # ground_truth[-1].append([train_vector, distance])
            ground_truth[-1].append([j, distance])
            ground_truth_l2[-1].append([j, l2_dist])
    
    for vector in ground_truth:
        vector.sort(key=lambda x: x[1])
    
    for vector in ground_truth_l2:
        vector.sort(key=lambda x: x[1])

    return ground_truth, ground_truth_l2

def write_ground_truth_to_file (ground_truth, ground_truth_l2):
    '''Write ground truth to file'''
    with open('ground_truth.txt', 'w') as f:
        for vector in ground_truth:
            for i in range(len(vector)):
                f.write(str(vector[i]))
                if i == len(vector)-1:
                    break
                f.write(',')
            f.write('\n')
            
    with open('ground_truth_l2.txt', 'w') as f:
        for vector in ground_truth_l2:
            for i in range(len(vector)):
                f.write(str(vector[i]))
                if i == len(vector)-1:
                    break
                f.write(',')
            f.write('\n')

def read_ground_truth_from_file (file_name):
    '''Read ground truth from file'''
    ground_truth = []
    distances = []
    with open(file_name, 'r') as f:
        for line in f:
            truth = []
            distance = []
            line = line.strip()
            temp = eval(line)
            for i in range(len(temp)):
                truth.append(temp[i][0])
                distance.append(temp[i][1])
                
            ground_truth.append(truth)
            distances.append(distance)
                
    return ground_truth, distances

if __name__ == '__main__':
    # train_data = read_vector_from_file('train_data.txt')
    # test_data = read_vector_from_file('test_data.txt')

    # train_filter = read_filter_from_file('train_filter.txt')
    # test_filter = read_filter_from_file('test_filter.txt')

    # print('train_data: ', train_data)
    # print('test_data: ', test_data)
    # print('train_filter: ', train_filter)
    # print('test_filter: ', test_filter)

    dir_path_orig = '/home/g6/Filters/FCSANN/Filters/Data'

    train_data = read_data_from_file(dir_path_orig + '/train_data.csv')
    test_data = read_data_from_file(dir_path_orig + '/test_data.csv')
    train_filter = read_filters_from_file(dir_path_orig + '/train_filters.csv')
    test_filter = read_filters_from_file(dir_path_orig + '/test_filters.csv')
    
    # get first 10 elements in test_data
    test_data = test_data[:2000]
    test_filter = test_filter[:2000]
    
        
    # data = [[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12], [13, 14, 15], [16, 17, 18], [19, 20, 21], [22, 23, 24], [25, 26, 27], [28, 29, 30]]
    # data_filters = [[[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[10], [9], [1], [2], [8]], [[10], [9], [1], [2], [8]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]], [[1], [3], [8], [9], [6]]]
    # query_point = [[15, 16, 17]]
    # query_point_filter = [[[1], [3], [8], [9], [6]]]
    
    print("Number of train data:", len(train_data))
    print("Number of test data:", len(test_data))
    print("Dimension of data points:", len(train_data[0]))
    print("Number of filters per data point:", len(train_filter[0]))
    print("Dimension of filters:", len(train_filter[0][0]))
    
    now = time.time()
    g, l2 = generate_ground_truth(train_data, test_data, train_filter, test_filter)
    print(time.time() - now)
    write_ground_truth_to_file(g, l2)
    # print(g)
