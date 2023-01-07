#ifndef FAKSULO_H
#define FAKSULO_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include "inc/Core/SearchQuery.h"

#define MxVectorSize 20000

class inverted_index_node {
    public:
        inverted_index_node(){
            cluster_id = -1;
        }
        inverted_index_node(int _cluster_id, std::vector<SPTAG::QueryResult*>& _query_results){
            cluster_id = _cluster_id;
            query_results = &_query_results;
        }
        int get_cluster_id() const{
            return cluster_id;
        }
        void set_cluster_id(int id){
            cluster_id = id;
        }
        std::vector<SPTAG::QueryResult*>& get_query_results() {
            return *(query_results);
        }
        void set_query_results(std::vector<SPTAG::QueryResult*>& _query_results){
            query_results = &_query_results;
        }

    private:
        int cluster_id;
        std::vector<SPTAG::QueryResult*>* query_results;
};

class input_query {
    public:
        input_query(){
            query_id = -1;
            nearest_clusters = {};
        }

        input_query(int _query_id, SPTAG::QueryResult& _query_result ,std::vector<int>& _nearest_clusters){
            query_id = _query_id;
            query_result = &_query_result;
            nearest_clusters = _nearest_clusters;
        }

        int get_query_id(){
            return query_id;
        }

        SPTAG::QueryResult* get_query_result(){
            return query_result;
        }
        
        void set_query_id(int id){
            query_id = id;
        }

        std::vector<int>& get_nearest_clusters(){
            return nearest_clusters;
        }
        
    private:
        int query_id;
        SPTAG::QueryResult* query_result;
        std::vector<int> nearest_clusters;
};

class Fakasulo_LL{
    public:
    Fakasulo_LL()
    {
            input_queries = nullptr;
            inverted_index = {};
    }

    Fakasulo_LL(std::vector<input_query>& _input_queries){
            input_queries = &_input_queries;
            std::map<int,int> centroid_count;
            for (input_query& q : _input_queries){
                for(auto cluster_id : q.get_nearest_clusters()){
                    if(inverted_index_map.find(cluster_id) == inverted_index_map.end()){

                        // Add in the inverted_index_map an empty list of vectors with cluster_id as key
                        inverted_index_map[cluster_id] = new std::list<std::vector<SPTAG::QueryResult*>>();
                        inverted_index_map[cluster_id]->push_back(std::vector<SPTAG::QueryResult*>());
                        inverted_index_map[cluster_id]->back().reserve(MxVectorSize);
                    }

                    if(centroid_count.count(cluster_id) == 0)
                        centroid_count[cluster_id] = 1;
                    else
                        centroid_count[cluster_id]++;
                }
            }
        }
    void process(){
            for(input_query& q : *(input_queries)){
                for(auto cluster_id : q.get_nearest_clusters()){
                    // check whether the vector is full with MxVectorSize elements
                    if(inverted_index_map[cluster_id]->back().size() >= MxVectorSize){
                        inverted_index_map[cluster_id]->push_back(std::vector<SPTAG::QueryResult*>());
                        inverted_index_map[cluster_id]->back().reserve(MxVectorSize);
                    }
                    inverted_index_map[cluster_id]->back().push_back(q.get_query_result());
                }
            }
            
            convert_map_to_vector();
            // for (auto x : inverted_index_map){
            //     // std::cout << "Centroid: " << x.first << " number of queries: " << inverted_index_map[x.first]->size() << std::endl;
            //     inverted_index_map[x.first]->shrink_to_fit();
            // }
        }
    
        void convert_map_to_vector(){
            
            inverted_index.clear();
            for(auto element : this->inverted_index_map){            
                inverted_index.emplace_back(element.first);
            }
        }

        std::map<int, std::list<std::vector<SPTAG::QueryResult*>>*>& get_inverted_index_map(){
            return inverted_index_map;
        }

    std::vector<int>& get_inverted_index(){
        return inverted_index;
    }
    ~Fakasulo_LL(){
        // for (auto i : inverted_index_map) {
        //     delete i.second; // we should replace it with shared pointers
        // }
    }


    private:
        std::vector<input_query>* input_queries;
        std::vector<int> inverted_index;
        std::map<int, std::list<std::vector<SPTAG::QueryResult*>>*> inverted_index_map;

};


class Fakasulo {
    public:
    Fakasulo(){
            input_queries = nullptr;
            inverted_index = {};
        }
    Fakasulo(std::vector<input_query>& _input_queries){
            input_queries = &_input_queries;
            std::map<int,int> centroid_count;
            for (input_query& q : _input_queries){
                for(auto cluster_id : q.get_nearest_clusters()){
                    if(inverted_index_map.find(cluster_id) == inverted_index_map.end()){
                        inverted_index_map[cluster_id] = new std::vector<SPTAG::QueryResult*>();
                        
                    }
                    if(centroid_count.count(cluster_id) == 0)
                        centroid_count[cluster_id] = 1;
                    else
                        centroid_count[cluster_id]++;
                }
            }
            // for (auto x : inverted_index_map){
            //     inverted_index_map[x.first]->reserve(4000000 * 5 * centroid_count[x.first] / (0.2 * 2000000 ));
            // }
        }
        
        void process(){
            for(input_query& q : *(input_queries)){
                for(auto cluster_id : q.get_nearest_clusters()){
                    inverted_index_map[cluster_id]->push_back(q.get_query_result());
                }
            }
            
            convert_map_to_vector();
            for (auto x : inverted_index_map){
                // std::cout << "Centroid: " << x.first << " number of queries: " << inverted_index_map[x.first]->size() << std::endl;
                inverted_index_map[x.first]->shrink_to_fit();
            }
        }
    
        void convert_map_to_vector(){
            inverted_index.clear();
            for(auto element : this->inverted_index_map){            
                inverted_index.emplace_back(element.first, *(element.second));
            }
        }

        std::map<int, std::vector<SPTAG::QueryResult*>*>& get_inverted_index_map(){
            return inverted_index_map;
        }

    std::vector<inverted_index_node>& get_inverted_index(){
        return inverted_index;
    }
        ~Fakasulo(){
            for (auto i : inverted_index_map) {
                delete i.second; // we should replace it with shared pointers
            }
        }

    private:
        std::vector<input_query>* input_queries;
        std::vector<inverted_index_node> inverted_index;

        std::map<int, std::vector<SPTAG::QueryResult*>*> inverted_index_map;

};

#endif
