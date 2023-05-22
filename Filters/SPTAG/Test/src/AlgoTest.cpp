// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/Test.h"
#include "inc/Helper/SimpleIniReader.h"
#include "inc/Core/VectorIndex.h"
#include "inc/Core/Common/CommonUtils.h"

#include <unordered_set>
#include <chrono>

template <typename T>
void Build(SPTAG::IndexAlgoType algo, std::string distCalcMethod, std::shared_ptr<SPTAG::VectorSet>& vec, std::shared_ptr<SPTAG::MetadataSet>& meta, const std::string out)
{

    std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
    BOOST_CHECK(nullptr != vecIndex);

    if (algo != SPTAG::IndexAlgoType::SPANN) {
        vecIndex->SetParameter("DistCalcMethod", distCalcMethod);
        vecIndex->SetParameter("NumberOfThreads", "16");
    }
    else {
        vecIndex->SetParameter("IndexAlgoType", "BKT", "Base");
        vecIndex->SetParameter("DistCalcMethod", distCalcMethod, "Base");

        vecIndex->SetParameter("isExecute", "true", "SelectHead");
        // vecIndex->SetParameter("NumberOfThreads", "4", "SelectHead");
        vecIndex->SetParameter("NumberOfThreads", "1", "SelectHead");

        vecIndex->SetParameter("Ratio", "0.2", "SelectHead"); // vecIndex->SetParameter("Count", "200", "SelectHead");

        vecIndex->SetParameter("isExecute", "true", "BuildHead");
        vecIndex->SetParameter("RefineIterations", "1", "BuildHead");
        // vecIndex->SetParameter("RefineIterations", "3", "BuildHead");
        vecIndex->SetParameter("NumberOfThreads", "1", "BuildHead");

        // vecIndex->SetParameter("NumberOfThreads", "4", "BuildHead");

        vecIndex->SetParameter("isExecute", "true", "BuildSSDIndex");
        vecIndex->SetParameter("BuildSsdIndex", "true", "BuildSSDIndex");
        // vecIndex->SetParameter("NumberOfThreads", "4", "BuildSSDIndex");
        vecIndex->SetParameter("NumberOfThreads", "64", "BuildSSDIndex");

        vecIndex->SetParameter("PostingPageLimit", "12", "BuildSSDIndex");
        vecIndex->SetParameter("SearchPostingPageLimit", "12", "BuildSSDIndex");
        vecIndex->SetParameter("InternalResultNum", "64", "BuildSSDIndex");
        vecIndex->SetParameter("SearchInternalResultNum", "64", "BuildSSDIndex");
    }

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndex(vec, meta));
    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
}

// template <typename T>
// void BuildWithMetaMapping(SPTAG::IndexAlgoType algo, std::string distCalcMethod, std::shared_ptr<SPTAG::VectorSet>& vec, std::shared_ptr<SPTAG::MetadataSet>& meta, const std::string out)
// {

//     std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
//     BOOST_CHECK(nullptr != vecIndex);

//     if (algo != SPTAG::IndexAlgoType::SPANN) {
//         vecIndex->SetParameter("DistCalcMethod", distCalcMethod);
//         vecIndex->SetParameter("NumberOfThreads", "16");
//     }
//     else {
//         vecIndex->SetParameter("IndexAlgoType", "BKT", "Base");
//         vecIndex->SetParameter("DistCalcMethod", distCalcMethod, "Base");

//         vecIndex->SetParameter("isExecute", "true", "SelectHead");
//         vecIndex->SetParameter("NumberOfThreads", "4", "SelectHead");
//         vecIndex->SetParameter("Ratio", "0.2", "SelectHead"); // vecIndex->SetParameter("Count", "200", "SelectHead");

//         vecIndex->SetParameter("isExecute", "true", "BuildHead");
//         vecIndex->SetParameter("RefineIterations", "3", "BuildHead");
//         vecIndex->SetParameter("NumberOfThreads", "4", "BuildHead");

//         vecIndex->SetParameter("isExecute", "true", "BuildSSDIndex");
//         vecIndex->SetParameter("BuildSsdIndex", "true", "BuildSSDIndex");
//         vecIndex->SetParameter("NumberOfThreads", "4", "BuildSSDIndex");
//         vecIndex->SetParameter("PostingPageLimit", "12", "BuildSSDIndex");
//         vecIndex->SetParameter("SearchPostingPageLimit", "12", "BuildSSDIndex");
//         vecIndex->SetParameter("InternalResultNum", "64", "BuildSSDIndex");
//         vecIndex->SetParameter("SearchInternalResultNum", "64", "BuildSSDIndex");
//     }

//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndex(vec, meta, true));
//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
// }

template <typename T>
void Search(const std::string folder, T* vec, SPTAG::SizeType n, int k, std::string* truthmeta)
{
    std::shared_ptr<SPTAG::VectorIndex> vecIndex;
    BOOST_CHECK(SPTAG::ErrorCode::Success == SPTAG::VectorIndex::LoadIndex(folder, vecIndex));
    BOOST_CHECK(nullptr != vecIndex);

    for (SPTAG::SizeType i = 0; i < n; i++) 
    {
        SPTAG::QueryResult res(vec, k, false);
        vecIndex->SearchIndexWFilters(res);
        // std::unordered_set<std::string> resmeta;
        // for (int j = 0; j < k; j++)
        // {
        //     // resmeta.insert(std::string((char*)res.GetMetadata(j).Data(), res.GetMetadata(j).Length()));
        //     // std::cout << res.GetResult(j)->Dist << "@(" << res.GetResult(j)->VID << "," << res.GetResult(j)->Filters[0]<< ") ";
            
        //     // std::cout << res.GetResult(j)->Dist << "@(" << res.GetResult(j)->VID << "," << std::string((char*)res.GetMetadata(j).Data(), res.GetMetadata(j).Length()) << ") ";
        // }
        std::cout << std::endl;
        // for (int j = 0; j < k; j++)
        // {
        //     BOOST_CHECK(resmeta.find(truthmeta[i * k + j]) != resmeta.end());
        // }
        vec += vecIndex->GetFeatureDim();
    }
    vecIndex.reset();
}

// template <typename T>
// void Add(const std::string folder, std::shared_ptr<SPTAG::VectorSet>& vec, std::shared_ptr<SPTAG::MetadataSet>& meta, const std::string out)
// {
//     std::shared_ptr<SPTAG::VectorIndex> vecIndex;
//     BOOST_CHECK(SPTAG::ErrorCode::Success == SPTAG::VectorIndex::LoadIndex(folder, vecIndex));
//     BOOST_CHECK(nullptr != vecIndex);

//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->AddIndex(vec, meta));
//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
//     vecIndex.reset();
// }

// template <typename T>
// void AddOneByOne(SPTAG::IndexAlgoType algo, std::string distCalcMethod, std::shared_ptr<SPTAG::VectorSet>& vec, std::shared_ptr<SPTAG::MetadataSet>& meta, const std::string out)
// {
//     std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
//     BOOST_CHECK(nullptr != vecIndex);

//     vecIndex->SetParameter("DistCalcMethod", distCalcMethod);
//     vecIndex->SetParameter("NumberOfThreads", "16");
    
//     auto t1 = std::chrono::high_resolution_clock::now();
//     for (SPTAG::SizeType i = 0; i < vec->Count(); i++) {
//         SPTAG::ByteArray metaarr = meta->GetMetadata(i);
//         std::uint64_t offset[2] = { 0, metaarr.Length() };
//         std::shared_ptr<SPTAG::MetadataSet> metaset(new SPTAG::MemMetadataSet(metaarr, SPTAG::ByteArray((std::uint8_t*)offset, 2 * sizeof(std::uint64_t), false), 1));
//         SPTAG::ErrorCode ret = vecIndex->AddIndex(vec->GetVector(i), 1, vec->Dimension(), metaset, true);
//         if (SPTAG::ErrorCode::Success != ret) std::cerr << "Error AddIndex(" << (int)(ret) << ") for vector " << i << std::endl;
//     }
//     auto t2 = std::chrono::high_resolution_clock::now();
//     std::cout << "AddIndex time: " << (std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (float)(vec->Count())) << "us" << std::endl;
    
//     Sleep(10000);

//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
// }

// template <typename T>
// void Delete(const std::string folder, T* vec, SPTAG::SizeType n, const std::string out)
// {
//     std::shared_ptr<SPTAG::VectorIndex> vecIndex;
//     BOOST_CHECK(SPTAG::ErrorCode::Success == SPTAG::VectorIndex::LoadIndex(folder, vecIndex));
//     BOOST_CHECK(nullptr != vecIndex);

//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->DeleteIndex((const void*)vec, n));
//     BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
//     vecIndex.reset();
// }

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

std::vector<std::vector<float>> read_vector_from_file (std::string file_name) {
    std::ifstream file(file_name);
    std::vector<std::vector<float>> vectors;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::vector<float> vector;
            std::stringstream ss(line);
            std::string item;
            while (std::getline(ss, item, ',')) {
                vector.push_back(std::stof(item));
            }
            vectors.push_back(vector);
        }
        file.close();
    }
    return vectors;
}

std::vector<std::vector<std::vector<float>>> read_filter_from_file(std::string file_name) {
    std::ifstream file(file_name);
    std::vector<std::vector<std::vector<float>>> filters;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::vector<float>> filter;
            std::stringstream ss(line);
            std::string item;
            while (std::getline(ss, item, ']')) {
                if (!item.empty() && item[0] == ',') {
                    item = item.substr(1);
                }
                if (!item.empty() && (item[0] == '[' || item[1] == '[')) {
                    item = item.substr(1);
                }
                std::vector<float> row;
                std::stringstream ss_row(item);
                std::string row_item;
                while (std::getline(ss_row, row_item, ',')) {
                    // printf("Here %s\n", row_item.c_str());
                    try {
                        row.push_back(std::stof(row_item.c_str()));
                    } catch (std::invalid_argument e) {
                        std::cout << "Invalid argument: " << item << std::endl;
                    }
                }
                filter.push_back(row);
            }
            filters.push_back(filter);
        }
        file.close();
    }
    return filters;
}

template <typename T>
void Test(SPTAG::IndexAlgoType algo, std::string distCalcMethod)
{   
    SPTAG::SizeType n = 2000, q = 3;
    SPTAG::DimensionType data_dim = 10;
    int k = 3;
    std::vector<T> vec;
    for (SPTAG::SizeType i = 0; i < n; i++) {
        for (SPTAG::DimensionType j = 0; j < data_dim; j++) {
            vec.push_back((T)i);
        }
    }
    std::vector<T> filters;
    for (SPTAG::SizeType i = 0; i < n; i++) {
            filters.push_back((T)i);
    }
    
    std::vector<T> query;
    for (SPTAG::SizeType i = 0; i < q; i++) {
        for (SPTAG::DimensionType j = 0; j < data_dim; j++) {
            query.push_back((T)n*i*2);
            // query.push_back((T)i);
        }
    }
    std::vector<T> query_filters;
    for (SPTAG::SizeType i = 0; i < q; i++) {
        query_filters.push_back((T)i+10);
    }
    

    // printf("data_dim: %d, filter_dim: %d, filters_num: %d, data_num: %d \n", data_dim, filter_dim, filters_num, data_num);

    // n = 1;
    k = 10;

    std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
    BOOST_CHECK(nullptr != vecIndex);
    vecIndex->SetParameter("IndexAlgoType", "BKT", "Base");
    vecIndex->SetParameter("DistCalcMethod", distCalcMethod, "Base");

    vecIndex->SetParameter("NumberOfThreads", "1", "BuildHead");
    vecIndex->SetParameter("RefineIterations", "1", "BuildHead");

    vecIndex->SetParameter("isExecute", "true", "SelectHead");
    vecIndex->SetParameter("NumberOfThreads", "1", "SelectHead");
    vecIndex->SetParameter("Ratio", "0.2", "SelectHead"); // vecIndex->SetParameter("Count", "200", "SelectHead");

    vecIndex->SetParameter("isExecute", "true", "BuildHead");

    vecIndex->SetParameter("isExecute", "true", "BuildSSDIndex");
    vecIndex->SetParameter("BuildSsdIndex", "true", "BuildSSDIndex");
    vecIndex->SetParameter("NumberOfThreads", "1", "BuildSSDIndex");
    
    vecIndex->SetParameter("PostingPageLimit", "12", "BuildSSDIndex");
    vecIndex->SetParameter("SearchPostingPageLimit", "12", "BuildSSDIndex");
    vecIndex->SetParameter("InternalResultNum", "64", "BuildSSDIndex");
    vecIndex->SetParameter("SearchInternalResultNum", "64", "BuildSSDIndex");

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndexWFilters(vec.data(), n, data_dim, filters.data(), 1, 1));

    // save index
    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex("test_index"));

    std::shared_ptr<SPTAG::VectorIndex> vecIndexLoaded;
    auto ret = SPTAG::VectorIndex::LoadIndex("test_index", vecIndexLoaded);

    if (SPTAG::ErrorCode::Success != ret){
        printf("Load index failed \n");
    } else {
        printf("Load index success \n");
    }

    T* vec1 = query.data();

    SPTAG::QueryResult res(vec1, k, false);
    res.InitFilters(query_filters.data(), 1, 1);
    // SPTAG::QueryResult res(query_point.data(), k, false);
    // res.InitFilters(query_point_filter.data(), filters_num, filter_dim);
    if (vecIndexLoaded->SearchIndexWFilters(res)==SPTAG::ErrorCode::Success){
        printf("Search with filters success \n");
    }
    else{
        printf("Search with filters failed \n");
    }

}

template <typename T>
void TestOld(SPTAG::IndexAlgoType algo, std::string distCalcMethod)
{

    SPTAG::SizeType n = 2000, q = 3;
    SPTAG::DimensionType m = 10;
    int k = 3;
    // std::vector<T> vec;
    // for (SPTAG::SizeType i = 0; i < n; i++) {
    //     for (SPTAG::DimensionType j = 0; j < m; j++) {
    //         vec.push_back((T)i);
    //     }
    // }
    
    // std::vector<T> query;
    // for (SPTAG::SizeType i = 0; i < q; i++) {
    //     for (SPTAG::DimensionType j = 0; j < m; j++) {
    //         query.push_back((T)n*i*2);
    //         // query.push_back((T)i);
    //     }
    // }

    // std::vector<T> query_filters;
    // for (SPTAG::SizeType i = 0; i < q; i++) {
    //     query_filters.push_back((T)i+10);
    // }
    
    // std::vector<T> filters;
    // for (SPTAG::SizeType i = 0; i < n; i++) {
    //         filters.push_back((T)i);
    // }
    
    // std::vector<std::vector<float>> vectors = read_vector_from_file("./FiltersTesting/train_data.txt");
    // std::vector<std::vector<std::vector<float>>> filters = read_filter_from_file("./FiltersTesting/train_filter.txt");

    // std::vector<char> meta;
    // std::vector<std::uint64_t> metaoffset;
    // for (SPTAG::SizeType i = 0; i < n; i++) {
    //     metaoffset.push_back((std::uint64_t)meta.size());
    //     std::string a = std::to_string(i);
    //     for (size_t j = 0; j < a.length(); j++)
    //         meta.push_back(a[j]);
    // }
    // metaoffset.push_back((std::uint64_t)meta.size());

    // std::shared_ptr<SPTAG::VectorSet> vecset(new SPTAG::BasicVectorSet(
    //     SPTAG::ByteArray((std::uint8_t*)vec.data(), sizeof(T) * n * m, false),
    //     SPTAG::GetEnumValueType<T>(), m, n));

    // std::shared_ptr<SPTAG::MetadataSet> metaset(new SPTAG::MemMetadataSet(
    //     SPTAG::ByteArray((std::uint8_t*)meta.data(), meta.size() * sizeof(char), false),
    //     SPTAG::ByteArray((std::uint8_t*)metaoffset.data(), metaoffset.size() * sizeof(std::uint64_t), false),
    //     n));
    // Search
    // Build<T>(algo, distCalcMethod, vecset, metaset, "testindicSearches");

    std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
    BOOST_CHECK(nullptr != vecIndex);
    vecIndex->SetParameter("IndexAlgoType", "BKT", "Base");
    vecIndex->SetParameter("DistCalcMethod", distCalcMethod, "Base");

        vecIndex->SetParameter("NumberOfThreads", "1", "BuildHead");
        vecIndex->SetParameter("RefineIterations", "1", "BuildHead");

    vecIndex->SetParameter("isExecute", "true", "SelectHead");
    vecIndex->SetParameter("NumberOfThreads", "1", "SelectHead");
    vecIndex->SetParameter("Ratio", "0.2", "SelectHead"); // vecIndex->SetParameter("Count", "200", "SelectHead");

    vecIndex->SetParameter("isExecute", "true", "BuildHead");
    // vecIndex->SetParameter("RefineIterations", "3", "BuildHead");
    // vecIndex->SetParameter("NumberOfThreads", "4", "BuildHead");

    vecIndex->SetParameter("isExecute", "true", "BuildSSDIndex");
    vecIndex->SetParameter("BuildSsdIndex", "true", "BuildSSDIndex");
    vecIndex->SetParameter("NumberOfThreads", "1", "BuildSSDIndex");
    
    vecIndex->SetParameter("PostingPageLimit", "12", "BuildSSDIndex");
    vecIndex->SetParameter("SearchPostingPageLimit", "12", "BuildSSDIndex");
    vecIndex->SetParameter("InternalResultNum", "64", "BuildSSDIndex");
    vecIndex->SetParameter("SearchInternalResultNum", "64", "BuildSSDIndex");



    std::string truthmeta1[] = { "0", "1", "2", "2", "1", "3", "4", "3", "5" };
    // Search<T>("testindices", query.data(), q, k, truthmeta1);

    // T* vec1 = query.data();
    
    // const int data_dim = 5; //5
    // const int filter_dim = 2;
    // const int filters_num = 3; //5
    // const int data_num = 10;
    // float pX[data_dim] = {17, 7, 2};
    // float pY[data_dim] = {1,3, 3};
    // float px_fnum[filters_num][filter_dim] = {{1}, {3}, {8}, {9}, {6}};
    // float py_fnum[filters_num][filter_dim] = {{1}, {7}, {8}, {9}, {10}};

    // float data[data_num][data_dim] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}, {16,17,18}, {19,20,21}, {22,23,24}, {25,26,27}, {28,29,30}};
    // float data_filters [data_num][filters_num][filter_dim] = {{{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{10, 1}, {9, 1}, {1, 1}, {2, 1}, {8, 1}}, {{10, 1}, {9, 1}, {1, 1}, {2, 1}, {8, 1}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}, {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}}};
    // float query_point[data_dim] = {15,16,17};
    // float query_point_filter[filters_num][filter_dim] = {{1, 2}, {3, 2}, {8, 2}, {9, 2}, {6, 2}};
    
    // float data [data_num][data_dim] = {{11,6,16,72,59}, {100,20,95,53,48}, {8,6,0,43,24}, {55,57,54,28,67}, {84,49,45,44,62}, {9,15,53,20,63}, {39,79,31,85,50}, {96,45,12,21,22}, {44,55,63,81,66}, {79,53,9,54,95}};
    // float data_filters [data_num][filters_num][filter_dim] = {{{0, 2}, {5, 5}, {0, 1}}, {{5, 4}, {4, 0}, {5, 0}}, {{5, 4}, {5, 1}, {3, 2}}, {{0, 0}, {2, 2}, {4, 5}}, {{4, 0}, {4, 5}, {0, 2}}, {{1, 3}, {4, 3}, {5, 0}}, {{1, 1}, {1, 4}, {4, 0}}, {{2, 3}, {0, 0}, {1, 4}}, {{5, 3}, {1, 1}, {2, 3}}, {{5, 5}, {5, 0}, {4, 4}}};

    // float query_point[data_dim] = {56,85,19,13,49};
    // float query_point_filter[filters_num][filter_dim] = {{0, 3}, {4, 3}, {5, 3}};

    std::vector<std::vector<float>> data = read_vector_from_file("./FiltersTesting/train_data.txt");
    std::vector<std::vector<std::vector<float>>> data_filters = read_filter_from_file("./FiltersTesting/train_filter.txt");
    std::vector<std::vector<float>> query_point = read_vector_from_file("./FiltersTesting/test_data.txt");
    std::vector<std::vector<std::vector<float>>> query_point_filter = read_filter_from_file("./FiltersTesting/test_filter.txt");

    const int data_dim = data[0].size();
    const int filter_dim = data_filters[0][0].size();
    const int filters_num = data_filters[0].size(); //5
    const int data_num = data.size();

    printf("data_dim: %d, filter_dim: %d, filters_num: %d, data_num: %d \n", data_dim, filter_dim, filters_num, data_num);

    n = 1;
    k = 10;

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndexWFilters(data.data(), data_num, data_dim, data_filters.data(), filters_num, filter_dim));
    // BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndexWFilters(data, data_num, data_dim, data_filters, filters_num, filter_dim));
    
    // T* vec1 = query_point.data();
    for (SPTAG::SizeType i = 0; i < n; i++) 
        {
            // SPTAG::QueryResult res(vec1, k, false);
            // res.InitFilters(query_filters.data(), 1, 1);
            SPTAG::QueryResult res(query_point.data(), k, false);
            res.InitFilters(query_point_filter.data(), filters_num, filter_dim);
            if (vecIndex->SearchIndexWFilters(res)==SPTAG::ErrorCode::Success){
                printf("Search with filters success \n");
                SPTAG::BasicResult* temp = res.GetResults();
                for (int j=0; j<k; j++){
                    printf("%d: %f\n", temp->VID, temp->Dist);
                    temp ++;
                }
            }
            else{
                printf("Search with filters failed \n");
            }
            // vecIndex->SearchIndexWFilters(res);
            // b AlgoTest.cpp: 258
            // p res.m_results
            std::cout << std::endl;
            // vec1 += vecIndex->GetFeatureDim();
        }
    printf("Search with filters success \n");


    // if (algo != SPTAG::IndexAlgoType::SPANN) {
    //     Add<T>("testindices", vecset, metaset, "testindices");
    //     std::string truthmeta2[] = { "0", "0", "1", "2", "2", "1", "4", "4", "3" };
    //     Search<T>("testindices", query.data(), q, k, truthmeta2);

    //     Delete<T>("testindices", query.data(), q, "testindices");
    //     std::string truthmeta3[] = { "1", "1", "3", "1", "3", "1", "3", "5", "3" };
    //     Search<T>("testindices", query.data(), q, k, truthmeta3);
    // }

    // BuildWithMetaMapping<T>(algo, distCalcMethod, vecset, metaset, "testindices");
    // std::string truthmeta4[] = { "0", "1", "2", "2", "1", "3", "4", "3", "5" };
    // Search<T>("testindices", query.data(), q, k, truthmeta4);

    // if (algo != SPTAG::IndexAlgoType::SPANN) {
    //     Add<T>("testindices", vecset, metaset, "testindices");
    //     std::string truthmeta5[] = { "0", "1", "2", "2", "1", "3", "4", "3", "5" };
    //     Search<T>("testindices", query.data(), q, k, truthmeta5);

    //     AddOneByOne<T>(algo, distCalcMethod, vecset, metaset, "testindices");
    //     std::string truthmeta6[] = { "0", "1", "2", "2", "1", "3", "4", "3", "5" };
    //     Search<float>("testindices", query.data(), q, k, truthmeta6);
    // }
}

BOOST_AUTO_TEST_SUITE (AlgoTest)

// BOOST_AUTO_TEST_CASE(KDTTest)
// {
//     Test<float>(SPTAG::IndexAlgoType::KDT, "L2");
// }

// BOOST_AUTO_TEST_CASE(BKTTest)
// {
//     Test<float>(SPTAG::IndexAlgoType::BKT, "L2");
// }

BOOST_AUTO_TEST_CASE(SPANNTest)
{
    printf("UPPPPPPPDATED\n");
    Test<float>(SPTAG::IndexAlgoType::SPANN, "L2");
}

BOOST_AUTO_TEST_SUITE_END()
