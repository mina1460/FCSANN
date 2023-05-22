# FCSANN- Accelerating Disk-Based Approximate Nearest Neighbor Search via Inverted Indexing

In This repository contains the source code for a document retrieval system built using the SPANN algorithm. The system consists of three main folders, each serving a specific purpose. Below is an overview of the folder structure and their functionalities.
Folder Structure

![System Architecture](https://example.com/path/to/image.jpg)

### SPTAG:
This folder contains the SPANN algorithm implementation with our extensions added to it. The SPANN algorithm utilizes both memory and disk storage to construct a hybrid index for efficient retrieval of nearest neighbors for a given query. Our extensions include an inverted-index-based, out-of-order query execution engine utilizing a producer-consumer threading pattern. It also includes the support for filter search on unstructured data.

### Embedding Unit:
The embedding unit folder contains the code for utilizing the SPANN engine to build a document retrieval system. It focuses on embedding documents into a high-dimensional space and performing nearest neighbor searches based on user queries. The embedding unit leverages the functionalities provided by the SPANN engine for efficient retrieval.

### Filter Search:
The filter search folder implements the filter search functionality on top of the SPANN model. This feature allows users to refine their search results by applying filters to their queries, narrowing down the results based on specific criteria. Filtered search improves the accuracy and speed of retrieval, especially in high-dimensional spaces.

# Context

This project aims to enhance the performance of approximate nearest neighbor search (ANNs) algorithms, specifically focusing on large-scale query batches and filter search on unstructured data. The SPANN algorithm, developed by Microsoft, serves as the foundation for our work. It efficiently handles massive datasets by utilizing both memory and disk storage.

Our extensions to the SPANN model include an inverted-index-based query execution engine that optimizes query processing and out-of-order execution for improved performance. Additionally, we introduce support for filter search on unstructured data, enabling users to apply filters to their queries and obtain more accurate results in less time.
How to Use

### To utilize this repository for similarity search, follow the instructions below:

    Clone the repository to your local machine using the command:

    bash

    git clone https://github.com/your-username/repository-name.git

    Engine: Navigate to the engine folder and follow the instructions provided in the README file to set up and use the SPANN engine with our extensions.

    Embedding Unit: Access the embedding-unit folder and refer to the README file for instructions on utilizing the SPANN engine to build a document retrieval system.

    Filter Search: Explore the filter-search folder and follow the instructions in the README file to implement and utilize the filter search functionality on top of the SPANN model.
    
 Datasets Used for testing and evaluation can be found in the following link:
 
 https://drive.google.com/drive/folders/1Z36hZSqfDA1hPRqBQNAx0CffwX-vNkdF

Please note that each folder contains its own set of instructions and dependencies. Make sure to review the respective README files for detailed guidance on setting up and running the code.
