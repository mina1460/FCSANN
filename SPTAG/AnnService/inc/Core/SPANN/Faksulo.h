#ifndef FAKSULO_H
#define FAKSULO_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include "inc/Core/SearchQuery.h"

typedef int CLUSTER_ID;
typedef int QUERY_ID;

// T is the cluster ID and U is the query ID
template <typename U, typename T>
class inverted_index_node {
    public:
        inverted_index_node(){
            cluster_id = -1;
            query_ids = {};
        }
        inverted_index_node(T _cluster_id, std::vector<U> _query_ids){
            cluster_id = _cluster_id;
            query_ids = _query_ids;
        }
        T get_cluster_id() const{
            return cluster_id;
        }
        void set_cluster_id(T id){
            cluster_id = id;
        }
        std::vector<U>& get_query_ids() {
            return query_ids;
        }
        void set_query_ids(std::vector<U> ids){
            query_ids = ids;
        }
    // void print_inverted_index_node(){
    //     std::cout << get_cluster_id() << " { " ;
    //     for(const auto cluster : get_query_ids()){
    //         std::cout << cluster << " ";
    //     }
    //     std::cout << "} \n";

    // }
    private:
        T cluster_id;
        std::vector<U> query_ids;
};

template <typename T, typename U>
class input_query {
    public:
        input_query(){
            query_id = -1;
            nearest_clusters = {};
        }
        input_query(int _query_id, std::vector<U> _nearest_clusters){
            query_id = _query_id;
            nearest_clusters = _nearest_clusters;
        }

        input_query(int _query_id, SPTAG::QueryResult _query_result ,std::vector<U> _nearest_clusters){
            query_id = _query_id;
            query_result = _query_result;
            nearest_clusters = _nearest_clusters;
        }

        T get_query_id(){
            return query_id;
        }

        SPTAG::QueryResult get_query_result(){
            return query_result;
        }
        
        void set_query_id(int id){
            query_id = id;
        }

        std::vector<U>& get_nearest_clusters(){
            return nearest_clusters;
        }
        
    // void print_query(){
    //     std::cout << get_query_id() << " { " ;
    //     for(auto cluster : get_nearest_clusters()){
    //         std::cout << cluster << " ";
    //     }
    //     std::cout << "} \n";

    // }
        

    private:
        T query_id;
        SPTAG::QueryResult query_result;
        std::vector<U> nearest_clusters;
};

// T --> QueryResult
// U --> ClusterID 
template <typename T, typename U>
class Fakasulo {
    public:
    Fakasulo(){
            input_queries = {};
            inverted_index = {};
        }
    Fakasulo(std::vector<input_query<T, U>> _input_queries){
            input_queries = _input_queries;
            for (input_query<T, U>& q : input_queries){
                for(auto& cluster_id : q.get_nearest_clusters()){
                    if(inverted_index_map.find(cluster_id) == inverted_index_map.end()){
                        inverted_index_map[cluster_id] = new std::vector<input_query<T, U>>();
                    }
                }
            }
        }
        
        void process(){
            for(input_query<T, U>& q : input_queries){
                for(auto cluster_id : q.get_nearest_clusters()){
                    if(inverted_index_map.find(cluster_id) == inverted_index_map.end()){
                        inverted_index_map[cluster_id] = new std::vector<input_query<T, U>>();
                    }
                    inverted_index_map[cluster_id]->push_back(q);
                }
            }
            
            convert_map_to_vector();
        }
    
        void convert_map_to_vector(){
            inverted_index.clear();
            for(auto element : this->inverted_index_map){
                std::vector<input_query<T, U>> temp;
                if(element.second){
                    temp = *(element.second);
                }
                inverted_index.emplace_back(element.first, temp);
            }
        }
    std::vector<inverted_index_node<T, U>> get_inverted_index(){
        return inverted_index;
    }

        ~Fakasulo(){
            // iterate over map and free the memory of the value
            for (auto i : inverted_index_map) {
                //std::cout << "Destructing the vector for cluster " << i.first << std::endl;
                delete i.second; // we should replace it with shared pointers
            }
        }

    private:
        std::vector<input_query<T, U>> input_queries;
        std::vector<inverted_index_node<T, U>> inverted_index;

        std::map<U, std::vector<input_query<T, U>>*> inverted_index_map;

};

#endif