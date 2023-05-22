#include <iostream>
#include <cstdint>
#include <cmath>
#include <queue>
using namespace std;
typedef std::int32_t DimensionType;
// #define W 0.25

float ComputeL2Distance(const float* pX, const float* pY, DimensionType dimension)
            {
                const float* pEnd4 = pX + ((dimension >> 2) << 2);
                const float* pEnd1 = pX + dimension;

                float diff = 0;

                while (pX < pEnd4) {
                    float c1 = ((float)(*pX++) - (float)(*pY++)); diff += c1 * c1;
                    c1 = ((float)(*pX++) - (float)(*pY++)); diff += c1 * c1;
                    c1 = ((float)(*pX++) - (float)(*pY++)); diff += c1 * c1;
                    c1 = ((float)(*pX++) - (float)(*pY++)); diff += c1 * c1;
                }
                while (pX < pEnd1) {
                    float c1 = ((float)(*pX++) - (float)(*pY++)); diff += c1 * c1;
                }
                return diff;
            }


float computeManhattanDistance(const float* pX, const float* pY, DimensionType dimension){
                const float* pEnd4 = pX + ((dimension >> 2) << 2);
                const float* pEnd1 = pX + dimension;

                float diff = 0;

                while (pX < pEnd4) {
                    float c1 = ((float)(*pX++) - (float)(*pY++)); diff += abs(c1);
                    c1 = ((float)(*pX++) - (float)(*pY++)); diff += abs(c1);
                    c1 = ((float)(*pX++) - (float)(*pY++)); diff += abs(c1);
                    c1 = ((float)(*pX++) - (float)(*pY++)); diff += abs(c1);
                }
                while (pX < pEnd1) {
                    float c1 = ((float)(*pX++) - (float)(*pY++)); diff += abs(c1);
                }
                return diff;
            }


float ComputeFiltersDistance(const float* pX_f, const float* pY_f, DimensionType dimension_f,DimensionType filtersNum, const float* pX, const float* pY, DimensionType dimension){
    float W=0.25;
    float filtersDistance = 0;
    for (int x=0; x<filtersNum; x++){
        int i=0;
        for(;i<dimension_f; i++){
            if(pX_f[i] != pY_f[i]){
                break;
                }        
        }
        if(i==dimension_f){
            continue;
        }
        else{
            float dist = ComputeL2Distance(pX, pY, dimension);
            float bias = 10*(W*dist+ 3.322);
            //this is not the exponential function. 
            // float expo= exp(ComputeL2Distance(pX_f, pY_f, dimension_f));    //wrong
            float expo = computeManhattanDistance(&pX_f[x], &pY_f[x], dimension_f);
            float logg= log10(expo+1);
            filtersDistance += bias- (1/logg);
            // return bias- (1/logg);
        }
    }   
        return filtersDistance/filtersNum;
}

float computeFusionDistance(float l2_dist, float fil_dist){
    float W=0.25;
    return W*l2_dist + fil_dist;
}

void printQueue(priority_queue<pair<float, int>> q){
    while(!q.empty()){
        cout<<q.top().second<<" "<<q.top().first<<endl;
        q.pop();
    }
}



int main(){
    const int data_dim = 3;
    const int filter_dim =1;
    const int filters_num = 5;
    const int data_num = 10;
    float pX[data_dim] = {17, 7, 2};
    float pY[data_dim] = {1,3, 3};
    //L2 distance between them is 5.477225575051661
    // float pX_f[filter_dim] = {1,2};
    // float py_f[filter_dim] = {1,2}; //4.0
    float px_fnum[filters_num][filter_dim] = {{1}, {3}, {8}, {9}, {6}};
    float py_fnum[filters_num][filter_dim] = {{1}, {7}, {8}, {9}, {10}};
    float data[data_num][data_dim] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}, {16,17,18}, {19,20,21}, {22,23,24}, {25,26,27}, {28,29,30}};
    float data_filters [data_num][filters_num][filter_dim] = {{{1}, {3}, {8}, {9}, {6}}, {{1}, {3}, {8}, {9}, {6}}, {{1}, {3}, {8}, {9}, {6}}, {{1}, {3}, {8}, {9}, {6}}, {{10}, {9}, {1}, {2}, {8}}, {{10}, {9}, {1}, {2}, {8}}, {{1}, {3}, {8}, {9}, {6}}, {{1}, {3}, {8}, {9}, {6}}, {{1}, {3}, {8}, {9}, {6}}, {{1}, {3}, {8}, {9}, {6}}};
    float query_point[data_dim] = {15,16,17};
    float query_point_filter[filters_num][filter_dim] = {{1}, {3}, {8}, {9}, {6}};
    //try multiple filters
    // float fil_dist=0.0; 
    // for(int j=0; j<filters_num; j++)
    // {
    //  fil_dist += ComputeFiltersDistance(px_fnum[j], py_fnum[j], filter_dim, pX, pY, data_dim);
    // }
    // fil_dist= fil_dist/filters_num;
    // float l2_dist =  ComputeL2Distance(pX, pY, data_dim);
    // float fusion_dist = computeFusionDistance(l2_dist, fil_dist);

    // printf("filter distnace is %f\n", fil_dist);
    // printf("L2 distance is %f\n",l2_dist );
    // printf("fusion distance is %f\n", fusion_dist);

    priority_queue<pair<float, int> > rank;
    for (int i =0; i<data_num; i++)
    {
        float fil_dist=0.0; 
        // for(int j=0; j<filters_num; j++)
        // {
         fil_dist = ComputeFiltersDistance(&query_point_filter[0][0], data_filters[i][0], filter_dim, filters_num,query_point, data[i], data_dim);
        // }
        // fil_dist= fil_dist/filters_num;
        float l2_dist =  ComputeL2Distance(query_point, data[i], data_dim);
        float fusion_dist = computeFusionDistance(l2_dist, fil_dist);
        rank.push(make_pair(fusion_dist, i));
    }

    cout<<"the queue is \n";
    printQueue(rank);

    return 0;
}