import SPTAG
import click
import os
import time
import shutil
import numpy as np
import pandas as pd



train = np.array([ [23, 48, 78, 14, 19], [123,78,7889,14,23], [1, 2, 3, 4, 5],[1, 2, 5,7,8]], np.float32)
train = np.append(train, np.random.rand(100,5).astype("float32"), axis=0)
filters = np.array([[1], [2],[7], [9]], np.float32)
filters = np.append(filters, np.random.rand(100,1).astype("float32"), axis=0)
test = np.array([[1,3,4,5,7]], np.float32)
result=[]
result_filters=[]
emp_arr=np.array([1])
num_elements =train.shape[0]
dim = train.shape[1]
filters_dim= filters.shape[1]
index = SPTAG.AnnIndex('SPANN', 'float', dim)



def construct_graph(algorithm):
    global index
    '''constructs the graph from the data'''
    click.echo(click.style('BUILDING GRAPH using ' + algorithm + ' ...', blink=True, bold=True))
    
    click.secho(f"{num_elements} elements, {dim} dimensions", fg='blue')
    build_time_start = time.time()    
    
    if algorithm == 'spann':
        if (os.path.exists("sptag_index")):
            index = SPTAG.AnnIndex.Load('sptag_index')
        else:   
            print("Begin SPANN") 
            # index = SPTAG.AnnIndex('SPANN', 'Float', dim)
            # index.SetBuildParam("NumberOfThreads", '4', "Index")
            # index.SetBuildParam("DistCalcMethod", 'L2', "Index")
            
            # index.SetBuildParam("ValueType", 'Float', "Base")
            index.SetBuildParam("DistCalcMethod", 'L2', "Base")
            index.SetBuildParam("IndexAlgoType", 'BKT', "Base")
            # index.SetBuildParam("NumberOfThreads", '4', "Base")
            # index.SetBuildParam("Dim", f"{dim}", "Base")
            
            # selecting the number of centroids depenfing on "ratio" of original data or "count"
            index.SetBuildParam("isExecute", 'true', "SelectHead")
            # index.SetBuildParam("TreeNumber", '1', "SelectHead")
            # index.SetBuildParam("BKTKmeansK", '32', "SelectHead")
            # index.SetBuildParam("BKTLeafSize", '8', "SelectHead")
            # index.SetBuildParam("SamplesNumber", '1000', "SelectHead")
            # index.SetBuildParam("SelectThreshold", '10', "SelectHead")
            # index.SetBuildParam("SplitFactor", '6', "SelectHead")
            # index.SetBuildParam("SplitThreshold", '25', "SelectHead")
            index.SetBuildParam("Ratio", '0.2', "SelectHead")
            index.SetBuildParam("NumberOfThreads", '1', "SelectHead")

            # 
            index.SetBuildParam("isExecute", 'true', "BuildHead")
            # index.SetBuildParam("NeighborhoodSize", '32', "BuildHead")
            # index.SetBuildParam("TPTNumber", '32', "BuildHead")
            # index.SetBuildParam("TPTLeafSize", '2000', "BuildHead")
            # index.SetBuildParam("MaxCheck", '16324', "BuildHead")
            # index.SetBuildParam("MaxCheckForRefineGraph", '16324', "BuildHead")
            index.SetBuildParam("RefineIterations", '1', "BuildHead")
            index.SetBuildParam("NumberOfThreads", '1', "BuildHead")

            index.SetBuildParam("isExecute", 'true', "BuildSSDIndex")
            index.SetBuildParam("BuildSsdIndex", 'true', "BuildSSDIndex")
            index.SetBuildParam("InternalResultNum", '64', "BuildSSDIndex")
            # index.SetBuildParam("ReplicaCount", '8', "BuildSSDIndex")
            index.SetBuildParam("PostingPageLimit", '12', "BuildSSDIndex")
            index.SetBuildParam("NumberOfThreads", '45', "BuildSSDIndex")
            # index.SetBuildParam("MaxCheck", '16324', "BuildSSDIndex")
            index.SetBuildParam("SearchInternalResultNum", '64', "BuildSSDIndex")
            index.SetBuildParam("SearchPostingPageLimit", '12', "BuildSSDIndex")

            # index.SetBuildParam("SearchResult", 'result.txt', "BuildSSDIndex")
            # index.SetBuildParam("ResultNum", '10', "BuildSSDIndex")
            # index.SetBuildParam("MaxDistRatio", '8.0', "BuildSSDIndex")

            if (os.path.exists("sptag_index_spann")):
                shutil.rmtree("sptag_index_spann")
            if index.BuildWithFilters(train,filters, num_elements, 1, filters_dim):
                index.Save("sptag_index_spann") # Save the index to the disk
            else:
                print("Build index failed!")
                

    build_time_end = time.time()
    click.secho(f"Build time: {(build_time_end - build_time_start):.3f} seconds\n", fg='bright_blue')


def query( algorithm) -> np.ndarray:
    global index
    '''queries the graph for the nearest neighbors'''
    query_time_start = time.time()
    if algorithm == 'spann':
        for i in test:
            result_filters.append(index.SearchWithFilters(i, emp_arr, 1, filters_dim, 2))
        for i in test:
            result.append (index.Search(i, 2))
        
        print(result)
        print(result_filters)
        

    
    query_time_end = time.time()

    click.secho("nearest neighbors: ", fg='green')
    click.secho(f"Query time: {(query_time_end - query_time_start):.4f} seconds\n", fg='bright_blue')
    # return labels

# print(train.dtype)
construct_graph('spann')
query('spann')