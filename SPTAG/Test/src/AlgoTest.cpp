// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/Test.h"
#include "inc/Helper/SimpleIniReader.h"
#include "inc/Core/VectorIndex.h"
#include "inc/Core/Common/CommonUtils.h"

#include <unordered_set>
#include <chrono>
#include <thread>

template <typename T>
void Build(SPTAG::IndexAlgoType algo, std::string distCalcMethod, std::shared_ptr<SPTAG::VectorSet> &vec, std::shared_ptr<SPTAG::MetadataSet> &meta, const std::string out)
{

    std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
    BOOST_CHECK(nullptr != vecIndex);

    if (algo != SPTAG::IndexAlgoType::SPANN)
    {
        vecIndex->SetParameter("DistCalcMethod", distCalcMethod);
        vecIndex->SetParameter("NumberOfThreads", "16");
    }
    else
    {
        vecIndex->SetParameter("IndexAlgoType", "BKT", "Base");
        vecIndex->SetParameter("DistCalcMethod", distCalcMethod, "Base");

        vecIndex->SetParameter("isExecute", "true", "SelectHead");
        vecIndex->SetParameter("NumberOfThreads", "4", "SelectHead");
        vecIndex->SetParameter("Ratio", "0.2", "SelectHead"); // vecIndex->SetParameter("Count", "200", "SelectHead");

        vecIndex->SetParameter("isExecute", "true", "BuildHead");
        vecIndex->SetParameter("RefineIterations", "3", "BuildHead");
        vecIndex->SetParameter("NumberOfThreads", "12", "BuildHead");

        vecIndex->SetParameter("isExecute", "true", "BuildSSDIndex");
        vecIndex->SetParameter("BuildSsdIndex", "true", "BuildSSDIndex");
        vecIndex->SetParameter("NumberOfThreads", "12", "BuildSSDIndex");
        vecIndex->SetParameter("PostingPageLimit", "12", "BuildSSDIndex");
        vecIndex->SetParameter("SearchPostingPageLimit", "12", "BuildSSDIndex");
        vecIndex->SetParameter("InternalResultNum", "5", "BuildSSDIndex");
        vecIndex->SetParameter("SearchInternalResultNum", "5", "BuildSSDIndex");
    }

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndex(vec, meta));
    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
}

template <typename T>
void BuildWithMetaMapping(SPTAG::IndexAlgoType algo, std::string distCalcMethod, std::shared_ptr<SPTAG::VectorSet> &vec, std::shared_ptr<SPTAG::MetadataSet> &meta, const std::string out)
{

    std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
    BOOST_CHECK(nullptr != vecIndex);

    if (algo != SPTAG::IndexAlgoType::SPANN)
    {
        vecIndex->SetParameter("DistCalcMethod", distCalcMethod);
        vecIndex->SetParameter("NumberOfThreads", "16");
    }
    else
    {
        vecIndex->SetParameter("IndexAlgoType", "BKT", "Base");
        vecIndex->SetParameter("DistCalcMethod", distCalcMethod, "Base");

        vecIndex->SetParameter("isExecute", "true", "SelectHead");
        vecIndex->SetParameter("NumberOfThreads", "4", "SelectHead");
        vecIndex->SetParameter("Ratio", "0.2", "SelectHead"); // vecIndex->SetParameter("Count", "200", "SelectHead");

        vecIndex->SetParameter("isExecute", "true", "BuildHead");
        vecIndex->SetParameter("RefineIterations", "3", "BuildHead");
        vecIndex->SetParameter("NumberOfThreads", "4", "BuildHead");

        vecIndex->SetParameter("isExecute", "true", "BuildSSDIndex");
        vecIndex->SetParameter("BuildSsdIndex", "true", "BuildSSDIndex");
        vecIndex->SetParameter("NumberOfThreads", "4", "BuildSSDIndex");
        vecIndex->SetParameter("PostingPageLimit", "12", "BuildSSDIndex");
        vecIndex->SetParameter("SearchPostingPageLimit", "12", "BuildSSDIndex");
        vecIndex->SetParameter("InternalResultNum", "64", "BuildSSDIndex");
        vecIndex->SetParameter("SearchInternalResultNum", "64", "BuildSSDIndex");
    }

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->BuildIndex(vec, meta, true));
    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
}

void ThreadedSearch(std::shared_ptr<SPTAG::VectorIndex> &vecIndex, std::vector<std::vector<SPTAG::QueryResult>> &queries, int i)
{
    vecIndex->SearchIndex(queries[i]);
}

bool SanityCheck(std::vector<SPTAG::QueryResult> FakasuloRes, std::vector<SPTAG::QueryResult> SptagRes, int k)
{
    std::cout << "Sanity Check \n";
    if (FakasuloRes.size() != SptagRes.size())
    {
        std::cout << "Results not size \n";
        return false;
    }
    int counter = 0;
    std::cout << "Res Size: " << FakasuloRes.size() << std::endl;
    std::cout << "Res Size SPTAG: " << SptagRes.size() << std::endl;
    for (int i = 0; i < FakasuloRes.size(); i++)
    {
        for (int j = 0; j < k; j++)
        {

            if (FakasuloRes[i].GetResult(j)->VID != SptagRes[i].GetResult(j)->VID)
            {
                std::cout << "Diff VID at Query: " << i << " at k neighbour: " << j << std::endl;
                std::cout << "SPTAG VID: " << SptagRes[i].GetResult(j)->VID << std::endl;
                std::cout << "Fakasulo VID: " << FakasuloRes[i].GetResult(j)->VID << std::endl;
                counter++;
            }
        }
    }
    std::cout << " NUM OF DIFF: " << counter << std::endl;
    if (counter == 0)
        return true;
    else
        return false;
}

template <typename T>
void Search(const std::string folder, T *vec, SPTAG::SizeType n, int k, std::string *truthmeta)
{
    int num_threads = 8;
    std::thread threadPool[num_threads];
    std::shared_ptr<SPTAG::VectorIndex> vecIndex;
    BOOST_CHECK(SPTAG::ErrorCode::Success == SPTAG::VectorIndex::LoadIndex(folder, vecIndex));
    BOOST_CHECK(nullptr != vecIndex);

    auto start_sptag_time = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<SPTAG::QueryResult>> queries(num_threads, std::vector<SPTAG::QueryResult>());
    std::vector<SPTAG::QueryResult> SptagRes;
    std::vector<SPTAG::QueryResult> FakasuloRes;
    for (SPTAG::SizeType i = 0; i < n; i++)
    {
        int index = i * num_threads / n;
        SPTAG::QueryResult res(vec, k, false);
        queries[index].push_back(res);
        /*
        vecIndex->SearchIndex(res);
        SptagRes.push_back(res);
        std::unordered_set<std::string> resmeta;
        // /*
        std::cout << "SPTAG query: " << i << "+";
        for (int j = 0; j < k; j++)
        {
            resmeta.insert(std::string((char*)res.GetMetadata(j).Data(), res.GetMetadata(j).Length()));
            std::cout << res.GetResult(j)->Dist << "@(" << res.GetResult(j)->VID << "," << std::string((char*)res.GetMetadata(j).Data(), res.GetMetadata(j).Length()) << ") ";
        }
        std::cout << std::endl;
        */

        vec += vecIndex->GetFeatureDim();
    }
    auto end_sptag_time = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_sptag_time - start_sptag_time);
    std::cout << "SPTAG time: " << int_ms.count() << std::endl;
    start_sptag_time = std::chrono::high_resolution_clock::now();

    vecIndex->ThreadedSelectHeads(queries, num_threads, 8);
    std::cout << "Finished\n";
    exit(0);
    // Our search
    for (int i = 0; i < num_threads; i++)
    {
        threadPool[i] = std::thread(ThreadedSearch, std::ref(vecIndex), std::ref(queries), i);
    }
    for (int i = 0; i < num_threads; i++)
    {
        threadPool[i].join();
    }

    end_sptag_time = std::chrono::high_resolution_clock::now();
    int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_sptag_time - start_sptag_time);
    std::cout << "Fakasulo time: " << int_ms.count() << std::endl;
    int our_index = 0;

    for (auto &queries_sub : queries)
    {
        for (auto &query : queries_sub)
        {
            std::cout << "Fakasulo query: " << our_index++ << "+";
            for (int j = 0; j < k; j++)
            {

                std::cout << query.GetResult(j)->Dist << "@(" << query.GetResult(j)->VID << ","
                          << ") ";
            }
            std::cout << "\n";
            FakasuloRes.push_back(query);
        }
    }

    std::cout << " ------------Sanity Check------------ \n";
    bool res = SanityCheck(FakasuloRes, SptagRes, k);

    vecIndex.reset();
}

template <typename T>
void Add(const std::string folder, std::shared_ptr<SPTAG::VectorSet> &vec, std::shared_ptr<SPTAG::MetadataSet> &meta, const std::string out)
{
    std::shared_ptr<SPTAG::VectorIndex> vecIndex;
    BOOST_CHECK(SPTAG::ErrorCode::Success == SPTAG::VectorIndex::LoadIndex(folder, vecIndex));
    BOOST_CHECK(nullptr != vecIndex);

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->AddIndex(vec, meta));
    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
    vecIndex.reset();
}

template <typename T>
void AddOneByOne(SPTAG::IndexAlgoType algo, std::string distCalcMethod, std::shared_ptr<SPTAG::VectorSet> &vec, std::shared_ptr<SPTAG::MetadataSet> &meta, const std::string out)
{
    std::shared_ptr<SPTAG::VectorIndex> vecIndex = SPTAG::VectorIndex::CreateInstance(algo, SPTAG::GetEnumValueType<T>());
    BOOST_CHECK(nullptr != vecIndex);

    vecIndex->SetParameter("DistCalcMethod", distCalcMethod);
    vecIndex->SetParameter("NumberOfThreads", "16");

    auto t1 = std::chrono::high_resolution_clock::now();
    for (SPTAG::SizeType i = 0; i < vec->Count(); i++)
    {
        SPTAG::ByteArray metaarr = meta->GetMetadata(i);
        std::uint64_t offset[2] = {0, metaarr.Length()};
        std::shared_ptr<SPTAG::MetadataSet> metaset(new SPTAG::MemMetadataSet(metaarr, SPTAG::ByteArray((std::uint8_t *)offset, 2 * sizeof(std::uint64_t), false), 1));
        SPTAG::ErrorCode ret = vecIndex->AddIndex(vec->GetVector(i), 1, vec->Dimension(), metaset, true);
        if (SPTAG::ErrorCode::Success != ret)
            std::cerr << "Error AddIndex(" << (int)(ret) << ") for vector " << i << std::endl;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "AddIndex time: " << (std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (float)(vec->Count())) << "us" << std::endl;

    Sleep(10000);

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
}

template <typename T>
void Delete(const std::string folder, T *vec, SPTAG::SizeType n, const std::string out)
{
    std::shared_ptr<SPTAG::VectorIndex> vecIndex;
    BOOST_CHECK(SPTAG::ErrorCode::Success == SPTAG::VectorIndex::LoadIndex(folder, vecIndex));
    BOOST_CHECK(nullptr != vecIndex);

    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->DeleteIndex((const void *)vec, n));
    BOOST_CHECK(SPTAG::ErrorCode::Success == vecIndex->SaveIndex(out));
    vecIndex.reset();
}

template <typename T>
void Test(SPTAG::IndexAlgoType algo, std::string distCalcMethod)
{

    SPTAG::SizeType n = 100000, q = 15000;
    SPTAG::DimensionType m = 100;
    int k = 5;
    std::vector<T> vec;
    // std::cout << "--------------Vec--------------------\n\n";
    for (SPTAG::SizeType i = 0; i < n; i++)
    {
        for (SPTAG::DimensionType j = 0; j < m; j++)
        {
            vec.push_back((T)(rand()));
            // vec.push_back((T)(i));
            //  std::cout << (T)i << " ";
        }
        // std::cout << std::endl;
    }
    // std::cout << "----------------------------------\n\n";

    std::vector<T> query;
    // std::cout << "----------------Query-----------------\n\n";
    for (SPTAG::SizeType i = 0; i < q; i++)
    {
        for (SPTAG::DimensionType j = 0; j < m; j++)
        {
            query.push_back((T)(rand()));
        }
        // std::cout << std::endl;
    }

    std::vector<char> meta;
    std::vector<std::uint64_t> metaoffset;
    for (SPTAG::SizeType i = 0; i < n; i++)
    {
        metaoffset.push_back((std::uint64_t)meta.size());
        std::string a = std::to_string(i);
        for (size_t j = 0; j < a.length(); j++)
            meta.push_back(a[j]);
    }
    metaoffset.push_back((std::uint64_t)meta.size());

    std::shared_ptr<SPTAG::VectorSet> vecset(new SPTAG::BasicVectorSet(
        SPTAG::ByteArray((std::uint8_t *)vec.data(), sizeof(T) * n * m, false),
        SPTAG::GetEnumValueType<T>(), m, n));

    std::shared_ptr<SPTAG::MetadataSet> metaset(new SPTAG::MemMetadataSet(
        SPTAG::ByteArray((std::uint8_t *)meta.data(), meta.size() * sizeof(char), false),
        SPTAG::ByteArray((std::uint8_t *)metaoffset.data(), metaoffset.size() * sizeof(std::uint64_t), false),
        n));

    // Build<T>(algo, distCalcMethod, vecset, metaset, "testindices_small");
    std::string truthmeta1[] = {"0", "1", "2", "2", "1", "3", "4", "3", "5"};
    // std::vector<T> debugQuery;
    // debugQuery.push_back(query[1]);
    // Search<T>("testindices_small", debugQuery.data(), 1, k, truthmeta1);
    Search<T>("testindices_small", query.data(), q, k, truthmeta1);
    exit(0);

    if (algo != SPTAG::IndexAlgoType::SPANN)
    {
        Add<T>("testindices", vecset, metaset, "testindices");
        std::string truthmeta2[] = {"0", "0", "1", "2", "2", "1", "4", "4", "3"};
        Search<T>("testindices", query.data(), q, k, truthmeta2);

        Delete<T>("testindices", query.data(), q, "testindices");
        std::string truthmeta3[] = {"1", "1", "3", "1", "3", "1", "3", "5", "3"};
        Search<T>("testindices", query.data(), q, k, truthmeta3);
    }

    BuildWithMetaMapping<T>(algo, distCalcMethod, vecset, metaset, "testindices");
    std::string truthmeta4[] = {"0", "1", "2", "2", "1", "3", "4", "3", "5"};
    Search<T>("testindices", query.data(), q, k, truthmeta4);

    if (algo != SPTAG::IndexAlgoType::SPANN)
    {
        Add<T>("testindices", vecset, metaset, "testindices");
        std::string truthmeta5[] = {"0", "1", "2", "2", "1", "3", "4", "3", "5"};
        Search<T>("testindices", query.data(), q, k, truthmeta5);

        AddOneByOne<T>(algo, distCalcMethod, vecset, metaset, "testindices");
        std::string truthmeta6[] = {"0", "1", "2", "2", "1", "3", "4", "3", "5"};
        Search<float>("testindices", query.data(), q, k, truthmeta6);
    }
}

BOOST_AUTO_TEST_SUITE(AlgoTest)

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
    Test<float>(SPTAG::IndexAlgoType::SPANN, "L2");
}

BOOST_AUTO_TEST_SUITE_END()
