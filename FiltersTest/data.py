from ground_truth import generate_ground_truth, write_ground_truth_to_file
import random
import time
import os
from PIL import Image
import pandas as pd

train_data_file = 'train_data.txt'
test_data_file = 'test_data.txt'
num_of_train_data = 100000
num_of_test_data = 10
data_dimension = 1024

train_filter_file = 'train_filter.txt'
test_filter_file = 'test_filter.txt'
filter_demension = 50
filter_num_per_vector = 5


def generate_data (filename, num_of_data):
    '''Generate vectors and save to file'''
    data = []
    with open(filename, 'w') as f:
        for i in range(num_of_data):
            tempArr=[]
            for j in range(data_dimension):
                x = random.randint(0, 100)
                # x = random.uniform(0, 100)
                tempArr.append(x)
                f.write(str(x))
                if j == data_dimension-1:
                    break
                f.write(',')
            f.write('\n') 
            data.append(tempArr)
    return data   

def generate_filters (filter_file, num_of_filter):
    '''Generate filters'''
    filters = []
    with open(filter_file, 'w') as f:
        for i in range(num_of_filter):
            tempArr=[]
            for j in range(filter_num_per_vector):
                tempArr.append([])
                for k in range(filter_demension):
                    tempArr[-1].append(random.randint(0, 5))
                f.write(str(tempArr[-1]))
                if j == filter_num_per_vector-1:
                    break
                f.write(',')
            f.write('\n')
            filters.append(tempArr)
    # print(filters[0:10])
    return filters

def load_images ():
    '''load images from file'''
    dir_path_orig = '/home/g6/Filters/FCSANN/Filters/Data'

    styles = pd.read_csv(dir_path_orig + '/styles_cleaned.csv', on_bad_lines='skip')
    styles = styles.dropna(subset=['baseColour', 'season'])

    data_path = dir_path_orig + '/images_compressed'

    data = []
    data_filters = []
    query = []
    query_filters = []
    count = 44000
    dimension = 1200
    for filename in os.listdir(data_path):
        if filename.endswith('.jpg') or filename.endswith('.png'): # Check if file is an image
            
            img_filters = styles.loc[styles['id'] == int(filename[:-4]), ['id', 'baseColour', 'season']]
            if img_filters.empty:
                continue
            temp_filters = []
            temp_filters.append([img_filters.baseColour.values[0]])
            temp_filters.append([img_filters.season.values[0]])

            img = Image.open(os.path.join(data_path, filename))
            pixels = [img.load()[x,y] for x in range(img.size[0]) for y in range(img.size[1])]
            if len(pixels) < dimension:
                pixels += [0] * (dimension - len(pixels))

            if count < 0:
                query.append(pixels)
                query_filters.append(temp_filters)
                if count == -300:
                    break                
            
            data.append(pixels)
            data_filters.append(temp_filters)
            count -= 1

    return data, data_filters, query, query_filters

if __name__ == '__main__':
    now = time.time()
    train_data = generate_data(train_data_file, num_of_train_data)
    print('Train data generation time: ', time.time()-now)
    now = time.time()
    test_data = generate_data(test_data_file, num_of_test_data)
    print('Test data generation time: ', time.time()-now)
    now = time.time()
    train_filter = generate_filters(train_filter_file, num_of_train_data)
    print('Train filter generation time: ', time.time()-now)
    now = time.time()
    test_filter = generate_filters(test_filter_file, num_of_test_data)    
    print('Test filter generation time: ', time.time()-now)

    # train_data, train_filter, test_data, test_filter = load_images()

    # print('Train data size: ', len(train_data))
    # print('Test data size: ', len(test_data))
    # print('Train filter size: ', len(train_filter))
    # print('Test filter size: ', len(test_filter))

    now = time.time()
    ground_truth, ground_truth_l2 = generate_ground_truth(train_data, test_data, train_filter, test_filter)
    print('Ground truth generation time: ', time.time()-now)
    write_ground_truth_to_file(ground_truth, ground_truth_l2)




