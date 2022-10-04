//
//  main.cpp
//  testing_fakasulo
//
//  Created by Mina Ashraf on 04/10/2022.
//

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

typedef int CLUSTER_ID;
typedef int QUERY_ID;

// T is the cluster ID and U is the query ID
template <typename T, typename U>
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
    void print_inverted_index_node(){
        std::cout << get_cluster_id() << " { " ;
        for(const auto cluster : get_query_ids()){
            std::cout << cluster << " ";
        }
        std::cout << "} \n";

    }
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
        input_query(T _query_id, std::vector<U> _nearest_clusters){
            query_id = _query_id;
            nearest_clusters = _nearest_clusters;
        }
        T get_query_id(){
            return query_id;
        }
        
        void set_query_id(T id){
            query_id = id;
        }

        std::vector<U>& get_nearest_clusters(){
            return nearest_clusters;
        }
        
    void print_query(){
        std::cout << get_query_id() << " { " ;
        for(auto cluster : get_nearest_clusters()){
            std::cout << cluster << " ";
        }
        std::cout << "} \n";

    }
        

    private:
        T query_id;
        std::vector<U> nearest_clusters;
};



class Fakasulo {
    public:
    Fakasulo(){
            input_queries = {};
            inverted_index = {};
        }
    Fakasulo(std::vector<input_query<QUERY_ID, CLUSTER_ID>> _input_queries){
            input_queries = _input_queries;
            for (input_query<QUERY_ID, CLUSTER_ID>& q : input_queries){
                for(auto& cluster_id : q.get_nearest_clusters()){
                    if(inverted_index_map.find(cluster_id) == inverted_index_map.end()){
                        inverted_index_map[cluster_id] = new std::vector<QUERY_ID>();
                    }
                }
            }
        }
        
        void process(){
            for(input_query<QUERY_ID, CLUSTER_ID>& q : input_queries){
                for(auto cluster_id : q.get_nearest_clusters()){
                    if(inverted_index_map.find(cluster_id) == inverted_index_map.end()){
                        inverted_index_map[cluster_id] = new std::vector<CLUSTER_ID>();
                    }
                    inverted_index_map[cluster_id]->push_back(q.get_query_id());
                }
            }
            
            convert_map_to_vector();
        }
    
        void convert_map_to_vector(){
            inverted_index.clear();
            for(auto element : this->inverted_index_map){
                std::vector<QUERY_ID> temp;
                if(element.second){
                    temp = *(element.second);
                }
                inverted_index.emplace_back(element.first, temp);
            }
        }
    std::vector<inverted_index_node<CLUSTER_ID, QUERY_ID>> get_inverted_index(){
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
        std::vector<input_query<QUERY_ID, CLUSTER_ID>> input_queries;
        std::vector<inverted_index_node<CLUSTER_ID, QUERY_ID>> inverted_index;

        std::map<CLUSTER_ID, std::vector<QUERY_ID>*> inverted_index_map;

};

int main(int argc, const char * argv[]) {
    std::vector<input_query<QUERY_ID, CLUSTER_ID>> input_queries;
    srand(time(NULL));
    
    int clusters_per_query = 3;
    int total_number_of_queries = 10000;
    int total_number_of_clusters = 0.16 * total_number_of_queries;

    
    for (int i=0; i<total_number_of_queries; i++) {
        std::vector<CLUSTER_ID> clusters;
        for(int j=0; j<clusters_per_query; j++){
            clusters.push_back(rand()%total_number_of_clusters + 1);
        }
        input_queries.push_back(input_query<QUERY_ID, CLUSTER_ID>(i, clusters));
    }
    /*
    for(auto query: input_queries){
        query.print_query();
    }
    */
    
    Fakasulo fakasulo(input_queries);
    std::cout << "Constructed Fakasulo \n";
    fakasulo.process();
    std::cout << "Finsihed processing  \n";
    
    std::vector<inverted_index_node<CLUSTER_ID, QUERY_ID>> inverted_index = fakasulo.get_inverted_index();
    for(auto element : inverted_index){
        std::cout << element.get_cluster_id() << " : ";
        for(auto vec : element.get_query_ids()) {
            std::cout << vec << " ";
        }
        std::cout << std::endl;
    }
    
    return 0;
}
