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
logging.basicConfig(level=logging.INFO, filename='./logs/ANN.log', format='%(asctime)s - {%(levelname)s} - %(message)s')


@dataclass
class Data:
    '''a representation of the data loaded from the hdf5 file'''
    distances: np.ndarray
    neighbors: np.ndarray
    train: np.ndarray
    test: np.ndarray
    distance: str

    def stats(self) -> None:
        '''return a string with some stats about the data'''
        click.echo("\n_______________________Stats_______________________\n")
        click.echo(click.style("Train:", bold=True) + f" {self.train.shape}" + f" -- This is the distances between all {self.train.shape[0]} vertices")
        click.echo(click.style("Distances:", bold=True) + f" {self.distances.shape}" + f" -- This is the distance to the nearest {self.distances.shape[1]} neighbors")
        click.echo(click.style("Neighbors:", bold=True) + f" {self.neighbors.shape}" + f" -- This is the indices of the nearest {self.distances.shape[1]} neighbors")
        
        click.echo(click.style("Test:", bold=True) + f" {self.test.shape}" + " -- This is the test data that we have the ground truth for")
        click.echo(click.style("Distance:", bold=True) + f" {self.distance}")
        click.echo("\n___________________________________________________\n")
        


class ANN:
    '''The main driver of everything, loads data, builds graphs, runs queries'''
    def __init__(self, filepath: str, n_neighbors: int, batch: bool, in_memory: bool) -> None:
        self.filepath = filepath
        self.n_neighbors = n_neighbors
        self.batch = batch
        self.in_memory = in_memory
        self.file: h5py.File
        self.data: Data
    
    def load_data(self, verbose=False) -> None:
        '''loads the data from the hdf5 file either in memory or out of memory'''
        driver = None
        if self.in_memory:
            driver = 'core'
        file = h5py.File(self.filepath, 'r', driver=driver)
        self.data = Data(
            distances=np.array(file.get('distances')),
            neighbors=np.array(file.get('neighbors')),
            train=np.array(file.get('train')),
            test=np.array(file.get('test')),
            distance=file.attrs['distance']
        )
            
        click.secho(f"Loaded data from {self.filepath}", fg='green')
        if verbose:
            self.data.stats()
            click.secho("\nDistances: ", fg='green')
            print(self.data.distances)
            click.secho("\nNeighbors: ", fg='green')
            print(self.data.neighbors)
            click.secho("\nTrain: ", fg='green')
            print(self.data.train)
            click.secho("\nTest: ", fg='green')
            print(self.data.test)

    def construct_graph(self, algorithm):
        '''constructs the graph from the data'''
        click.echo(click.style('BUILDING GRAPH using ' + algorithm + ' ...', blink=True, bold=True))
        num_elements = self.data.train.shape[0]
        dim = self.data.train.shape[1]
        click.secho(f"{num_elements} elements, {dim} dimensions", fg='blue')
        build_time_start = time.time()    
        
        if algorithm == 'spann':
            if (os.path.exists("sptag_index")):
                self.index = SPTAG.AnnIndex.Load('sptag_index')
            else:   
                print("Begin SPANN") 
                self.index = SPTAG.AnnIndex('SPANN', 'Float', dim)
                # self.index.SetBuildParam("NumberOfThreads", '4', "Index")
                # self.index.SetBuildParam("DistCalcMethod", 'L2', "Index")
                
                # self.index.SetBuildParam("ValueType", 'Float', "Base")
                self.index.SetBuildParam("DistCalcMethod", 'L2', "Base")
                self.index.SetBuildParam("IndexAlgoType", 'BKT', "Base")
                # self.index.SetBuildParam("NumberOfThreads", '4', "Base")
                # self.index.SetBuildParam("Dim", f"{dim}", "Base")
                
                # selecting the number of centroids depenfing on "ratio" of original data or "count"
                self.index.SetBuildParam("isExecute", 'true', "SelectHead")
                # self.index.SetBuildParam("TreeNumber", '1', "SelectHead")
                # self.index.SetBuildParam("BKTKmeansK", '32', "SelectHead")
                # self.index.SetBuildParam("BKTLeafSize", '8', "SelectHead")
                # self.index.SetBuildParam("SamplesNumber", '1000', "SelectHead")
                # self.index.SetBuildParam("SelectThreshold", '10', "SelectHead")
                # self.index.SetBuildParam("SplitFactor", '6', "SelectHead")
                # self.index.SetBuildParam("SplitThreshold", '25', "SelectHead")
                self.index.SetBuildParam("Ratio", '0.2', "SelectHead")
                self.index.SetBuildParam("NumberOfThreads", '4', "SelectHead")

                # 
                self.index.SetBuildParam("isExecute", 'true', "BuildHead")
                # self.index.SetBuildParam("NeighborhoodSize", '32', "BuildHead")
                # self.index.SetBuildParam("TPTNumber", '32', "BuildHead")
                # self.index.SetBuildParam("TPTLeafSize", '2000', "BuildHead")
                # self.index.SetBuildParam("MaxCheck", '16324', "BuildHead")
                # self.index.SetBuildParam("MaxCheckForRefineGraph", '16324', "BuildHead")
                self.index.SetBuildParam("RefineIterations", '3', "BuildHead")
                self.index.SetBuildParam("NumberOfThreads", '4', "BuildHead")

                self.index.SetBuildParam("isExecute", 'true', "BuildSSDIndex")
                self.index.SetBuildParam("BuildSsdIndex", 'true', "BuildSSDIndex")
                self.index.SetBuildParam("InternalResultNum", '64', "BuildSSDIndex")
                # self.index.SetBuildParam("ReplicaCount", '8', "BuildSSDIndex")
                self.index.SetBuildParam("PostingPageLimit", '12', "BuildSSDIndex")
                self.index.SetBuildParam("NumberOfThreads", '45', "BuildSSDIndex")
                # self.index.SetBuildParam("MaxCheck", '16324', "BuildSSDIndex")
                self.index.SetBuildParam("SearchInternalResultNum", '64', "BuildSSDIndex")
                self.index.SetBuildParam("SearchPostingPageLimit", '12', "BuildSSDIndex")
                # self.index.SetBuildParam("SearchResult", 'result.txt', "BuildSSDIndex")
                # self.index.SetBuildParam("ResultNum", '10', "BuildSSDIndex")
                # self.index.SetBuildParam("MaxDistRatio", '8.0', "BuildSSDIndex")

                if (os.path.exists("sptag_index_spann")):
                    shutil.rmtree("sptag_index_spann")
                if self.index.Build(self.data.train, num_elements, False):
                    self.index.Save("sptag_index_spann") # Save the index to the disk

        build_time_end = time.time()
        click.secho(f"Build time: {(build_time_end - build_time_start):.3f} seconds\n", fg='bright_blue')


    def query(self, algorithm) -> np.ndarray:
        '''queries the graph for the nearest neighbors'''
        query_time_start = time.time()
        result=[]
        if algorithm == 'spann':
            print(type(self.data.test))
            print(self.data.test.shape)
            for i in self.data.test:
                result.append (self.index.Search(i, self.n_neighbors)[0])
            df= pd.DataFrame(result)
            df.to_csv('results.csv')
            labels= result
        
        query_time_end = time.time()

        click.secho("nearest neighbors: ", fg='green')
        click.secho(f"Query time: {(query_time_end - query_time_start):.4f} seconds\n", fg='bright_blue')
        print (type(self.data.neighbors[:,:self.n_neighbors]))
        return labels

    def k_recall_at_k(self, labels, ground_truth) -> float:
        '''calculating the k recall at k'''
        return np.mean([(len(set(x).intersection(set(y))) / labels.shape[1]) for (x,y) in zip(labels,ground_truth)])

    def evaluate(self, labels) -> float:
        '''evaluates the recall of the query'''
        click.secho("evaluating recall: ", fg='green')
        print(self.data.neighbors[:,:self.n_neighbors].shape)
        print(len(labels))
        labels = np.array([np.array(i) for i in labels])
        k_recall_k = self.k_recall_at_k(labels, self.data.neighbors[:,:self.n_neighbors-1])
        click.secho(f"k-recall@k: {k_recall_k}", fg='red', bold=True)

        return 0


#get arguments from command line using click

@click.command()
@click.option('--dataset','-d', prompt='File path to dataset', type=click.Path(exists=True), required=True, help='The file to read.')
@click.option('--k_neighbors','-k', help='Number of neighbors', default=10, type=int)
@click.option('--batch', help='Enable batch processing', default=False, is_flag=True, type=bool)
@click.option('--in_memory', '-m', help='Load data in memory', default=True, is_flag=True, type=bool)
@click.option('--verbose', '-v', help='Enable verbose output', default=False, is_flag=True, type=bool)
@click.option('--algorithm', '-a', help='Algorithm to use', default='spann', type=click.Choice(['hnsw', 'nndescent', 'spann']))
def entry_point(dataset, k_neighbors, batch, in_memory, verbose, algorithm) -> None:
    '''the entry point of the program'''
    click.secho(f"\nstarted processing {os.path.basename(dataset)}\n", fg='blue')
    ann = ANN(dataset, k_neighbors, batch, in_memory)
    ann.load_data(verbose=verbose)
    ann.construct_graph(algorithm)
    labels = ann.query(algorithm)
    recall = ann.evaluate(labels)



if __name__ == '__main__':
    entry_point()
