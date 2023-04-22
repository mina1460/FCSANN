// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/CoreInterface.h"
#include "inc/Helper/StringConvert.h"
#include <chrono>
#include <thread>

AnnIndex::AnnIndex(DimensionType p_dimension)
    : m_algoType(SPTAG::IndexAlgoType::BKT),
      m_inputValueType(SPTAG::VectorValueType::Float),
      m_dimension(p_dimension)
{
    m_inputVectorSize = SPTAG::GetValueTypeSize(m_inputValueType) * m_dimension;
}


AnnIndex::AnnIndex(const char* p_algoType, const char* p_valueType, DimensionType p_dimension)
    : m_algoType(SPTAG::IndexAlgoType::Undefined),
      m_inputValueType(SPTAG::VectorValueType::Undefined),
      m_dimension(p_dimension)
{
    SPTAG::Helper::Convert::ConvertStringTo<SPTAG::IndexAlgoType>(p_algoType, m_algoType);
    SPTAG::Helper::Convert::ConvertStringTo<SPTAG::VectorValueType>(p_valueType, m_inputValueType);
    m_inputVectorSize = SPTAG::GetValueTypeSize(m_inputValueType) * m_dimension;
}


AnnIndex::AnnIndex(const std::shared_ptr<SPTAG::VectorIndex>& p_index)
    : m_algoType(p_index->GetIndexAlgoType()),
      m_inputValueType(p_index->GetVectorValueType()),
      m_dimension(p_index->GetFeatureDim()),
      m_index(p_index)
{
    m_inputVectorSize = p_index->m_pQuantizer ? p_index->m_pQuantizer->GetNumSubvectors() : SPTAG::GetValueTypeSize(m_inputValueType) * m_dimension;
}


AnnIndex::~AnnIndex()
{
}



bool
AnnIndex::BuildSPANN(bool p_normalized)
{
    if (nullptr == m_index)
    {
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }
    if (nullptr == m_index) return false;

    return (SPTAG::ErrorCode::Success == m_index->BuildIndex(p_normalized));
}

bool
AnnIndex::BuildSPANNWithMetaData(ByteArray p_meta, SizeType p_num, bool p_withMetaIndex, bool p_normalized)
{
    if (nullptr == m_index)
    {
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }
    if (nullptr == m_index) return false;

    std::uint64_t* offsets = new std::uint64_t[p_num + 1]{ 0 };
    if (!SPTAG::MetadataSet::GetMetadataOffsets(p_meta.Data(), p_meta.Length(), offsets, p_num + 1, '\n')) return false;

    m_index->SetMetadata((new SPTAG::MemMetadataSet(p_meta, ByteArray((std::uint8_t*)offsets, (p_num + 1) * sizeof(std::uint64_t), true), (SPTAG::SizeType)p_num,
        m_index->m_iDataBlockSize, m_index->m_iDataCapacity, m_index->m_iMetaRecordSize)));
    if (p_withMetaIndex) m_index->BuildMetaMapping(false);

    return (SPTAG::ErrorCode::Success == m_index->BuildIndex(p_normalized));
}


bool
AnnIndex::Build(ByteArray p_data, SizeType p_num, bool p_normalized)
{

    if (nullptr == m_index)
    {
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }
    if (nullptr == m_index || p_num == 0 || m_dimension == 0 || p_data.Length() != p_num * m_inputVectorSize)
    {
        std::cout << "Build failed----------------------------" << std::endl;
        std::cout << "p_num: " << p_num << std::endl;
        std::cout << "m_dimension: " << m_dimension << std::endl;
        std::cout << "p_data.Length(): " << p_data.Length() << std::endl;
        std::cout << "m_inputVectorSize: " << m_inputVectorSize << std::endl;

        return false;
    }
    return (SPTAG::ErrorCode::Success == m_index->BuildIndex(p_data.Data(), (SPTAG::SizeType)p_num, (SPTAG::DimensionType)m_dimension, p_normalized));
}


bool
AnnIndex::BuildWithMetaData(ByteArray p_data, ByteArray p_meta, SizeType p_num, bool p_withMetaIndex, bool p_normalized)
{
    if (nullptr == m_index)
    {
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }
    if (nullptr == m_index || p_num == 0 || m_dimension == 0 || p_data.Length() != p_num * m_inputVectorSize)
    {
        return false;
    }
    
    auto vectorType = m_index->m_pQuantizer ? SPTAG::VectorValueType::UInt8 : m_inputValueType;
    auto vectorSize = m_index->m_pQuantizer ? m_index->m_pQuantizer->GetNumSubvectors() : m_dimension;
    std::shared_ptr<SPTAG::VectorSet> vectors(new SPTAG::BasicVectorSet(p_data,
        vectorType,
        static_cast<SPTAG::DimensionType>(vectorSize),
        static_cast<SPTAG::SizeType>(p_num)));

    std::uint64_t* offsets = new std::uint64_t[p_num + 1]{ 0 };
    if (!SPTAG::MetadataSet::GetMetadataOffsets(p_meta.Data(), p_meta.Length(), offsets, p_num + 1, '\n')) return false;
    std::shared_ptr<SPTAG::MetadataSet> meta(new SPTAG::MemMetadataSet(p_meta, ByteArray((std::uint8_t*)offsets, (p_num + 1) * sizeof(std::uint64_t), true), (SPTAG::SizeType)p_num,
        m_index->m_iDataBlockSize, m_index->m_iDataCapacity, m_index->m_iMetaRecordSize));
    return (SPTAG::ErrorCode::Success == m_index->BuildIndex(vectors, meta, p_withMetaIndex, p_normalized));
}


void
AnnIndex::SetBuildParam(const char* p_name, const char* p_value, const char* p_section)
{
    if (nullptr == m_index) 
    {
        if (SPTAG::IndexAlgoType::Undefined == m_algoType || 
            SPTAG::VectorValueType::Undefined == m_inputValueType)
        {
            return;    
        }
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);

    }
    m_index->SetParameter(p_name, p_value, p_section);
}


void
AnnIndex::SetSearchParam(const char* p_name, const char* p_value, const char* p_section)
{
    if (nullptr != m_index) m_index->SetParameter(p_name, p_value, p_section);
}


bool
AnnIndex::LoadQuantizer(const char* p_quantizerFile)
{
    if (nullptr == m_index)
    {
        if (SPTAG::IndexAlgoType::Undefined == m_algoType ||
            SPTAG::VectorValueType::Undefined == m_inputValueType)
        {
            return false;
        }
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }

    auto ret = (m_index->LoadQuantizer(p_quantizerFile) == SPTAG::ErrorCode::Success);
    if (ret)
    {
        m_inputVectorSize = m_index->m_pQuantizer->QuantizeSize();
    }
    return ret;
}


void
AnnIndex::SetQuantizerADC(bool p_adc)
{
    if (nullptr != m_index) return m_index->SetQuantizerADC(p_adc);
}


std::shared_ptr<QueryResult>
AnnIndex::Search(ByteArray p_data, int p_resultNum)
{
    std::shared_ptr<QueryResult> results = std::make_shared<QueryResult>(p_data.Data(), p_resultNum, false);

    if (nullptr != m_index)
    {
        m_index->SearchIndex(*results);
    }
    return std::move(results);
}

std::shared_ptr<QueryResult>
AnnIndex::SearchWithMetaData(ByteArray p_data, int p_resultNum)
{
    std::shared_ptr<QueryResult> results = std::make_shared<QueryResult>(p_data.Data(), p_resultNum, true);

    if (nullptr != m_index)
    {
        m_index->SearchIndex(*results);
    }
    return std::move(results);
}

std::shared_ptr<QueryResult>
AnnIndex::BatchSearch(ByteArray p_data, int p_vectorNum, int p_resultNum, bool p_withMetaData)
{
    std::shared_ptr<QueryResult> results = std::make_shared<QueryResult>(p_data.Data(), p_vectorNum * p_resultNum, p_withMetaData);
    if (nullptr != m_index)
    {
        m_index->SearchIndex(p_data.Data(), p_vectorNum, p_resultNum, p_withMetaData, results->GetResults());
    }
    return std::move(results);
}

void ThreadedSearch(std::shared_ptr<SPTAG::VectorIndex> &vecIndex, std::vector<std::vector<SPTAG::QueryResult>> &queries, int i){
     vecIndex->SearchIndex(queries[i]);
}

std::vector<QueryResult>
AnnIndex::FakasuloSearch(ByteArray p_data, int p_vectorNum, int p_resultNum, bool p_withMetaData, int p_num_threads)
{ 

    int num_threads = p_num_threads;
    std::thread threadPool[num_threads];
    std::vector<std::vector<SPTAG::QueryResult>> queries(num_threads, std::vector<SPTAG::QueryResult>());

    std::vector<QueryResult> input_queries; 
    
    for (int i = 0; i < p_vectorNum; i++) {
        int index = i * num_threads / p_vectorNum; 
        queries[index].emplace_back(p_data.Data() + i * m_inputVectorSize, p_resultNum, p_withMetaData, i);
    }
    auto start = std::chrono::high_resolution_clock::now();

    for(int i=0; i<num_threads; i++){
        threadPool[i] = std::thread(ThreadedSearch, std::ref(m_index) ,std::ref(queries), i);
    }
    for(int i=0; i<num_threads; i++){
        threadPool[i].join();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    // if (nullptr != m_index)
    // {
    //     std::cout << "Start search \n";
    //     m_index->SearchIndex(input_queries, true);
    // }

    
    int our_index = 0;
    int k= p_resultNum;
    
    // for(auto& query: input_queries){
    // std::cout << "Fakasulo query: " << our_index++ << "+";
    //     for (int j = 0; j < k; j++)
    //     {
            
    //         std::cout << query.GetResult(j)->Dist << "@(" << query.GetResult(j)->VID << "," << query.GetResult(j)->VID << ") ";
    //     }
    //     std::cout << "\n";
    // }

    // Add the results to the input_queries
    int counter = 0 ;
    for(int i=0; i<num_threads; i++){
        for(auto& query: queries[i]){
            input_queries.push_back(query);
        }
    }


    return std::move(input_queries);
}

bool
AnnIndex::ReadyToServe() const
{
    return m_index != nullptr;
}


void
AnnIndex::UpdateIndex()
{
    m_index->UpdateIndex();
}


bool
AnnIndex::Save(const char* p_savefile) const
{
    std::cout << "save index\n";
    // print p_savefile
    std::cout << p_savefile << "\n";
    return SPTAG::ErrorCode::Success == m_index->SaveIndex(p_savefile);
}


AnnIndex
AnnIndex::Load(const char* p_loaderFile)
{
    std::shared_ptr<SPTAG::VectorIndex> vecIndex;
    auto ret = SPTAG::VectorIndex::LoadIndex(p_loaderFile, vecIndex);
    if (SPTAG::ErrorCode::Success != ret || nullptr == vecIndex)
    {
        return AnnIndex(0);
    }

    return AnnIndex(vecIndex);
}


bool 
AnnIndex::Add(ByteArray p_data, SizeType p_num, bool p_normalized)
{
    if (nullptr == m_index)
    {
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }
    if (nullptr == m_index || p_num == 0 || m_dimension == 0 || p_data.Length() != p_num * m_inputVectorSize)
    {
        return false;
    }

    return (SPTAG::ErrorCode::Success == m_index->AddIndex(p_data.Data(), (SPTAG::SizeType)p_num, (SPTAG::DimensionType)m_dimension, nullptr, false, p_normalized));
}


bool
AnnIndex::AddWithMetaData(ByteArray p_data, ByteArray p_meta, SizeType p_num, bool p_withMetaIndex, bool p_normalized)
{
    if (nullptr == m_index)
    {
        m_index = SPTAG::VectorIndex::CreateInstance(m_algoType, m_inputValueType);
    }
    if (nullptr == m_index || p_num == 0 || m_dimension == 0 || p_data.Length() != p_num * m_inputVectorSize)
    {
        return false;
    }

    std::shared_ptr<SPTAG::VectorSet> vectors(new SPTAG::BasicVectorSet(p_data,
        m_inputValueType,
        static_cast<SPTAG::DimensionType>(m_dimension),
        static_cast<SPTAG::SizeType>(p_num)));

    std::uint64_t* offsets = new std::uint64_t[p_num + 1]{ 0 };
    if (!SPTAG::MetadataSet::GetMetadataOffsets(p_meta.Data(), p_meta.Length(), offsets, p_num + 1, '\n')) return false;
    std::shared_ptr<SPTAG::MetadataSet> meta(new SPTAG::MemMetadataSet(p_meta, ByteArray((std::uint8_t*)offsets, (p_num + 1) * sizeof(std::uint64_t), true), (SPTAG::SizeType)p_num));
    return (SPTAG::ErrorCode::Success == m_index->AddIndex(vectors, meta, p_withMetaIndex, p_normalized));
}


bool
AnnIndex::Delete(ByteArray p_data, SizeType p_num)
{
    if (nullptr == m_index || p_num == 0 || m_dimension == 0 || p_data.Length() != p_num * m_inputVectorSize)
    {
        return false;
    }

    return (SPTAG::ErrorCode::Success == m_index->DeleteIndex(p_data.Data(), (SPTAG::SizeType)p_num));
}


bool
AnnIndex::DeleteByMetaData(ByteArray p_meta)
{
    if (nullptr == m_index) return false;
    
    return (SPTAG::ErrorCode::Success == m_index->DeleteIndex(p_meta));
}


AnnIndex
AnnIndex::Merge(const char* p_indexFilePath1, const char* p_indexFilePath2)
{
    std::shared_ptr<SPTAG::VectorIndex> vecIndex, addIndex;
    if (SPTAG::ErrorCode::Success != SPTAG::VectorIndex::LoadIndex(p_indexFilePath1, vecIndex) ||
        SPTAG::ErrorCode::Success != SPTAG::VectorIndex::LoadIndex(p_indexFilePath2, addIndex) ||
        SPTAG::ErrorCode::Success != vecIndex->MergeIndex(addIndex.get(), std::atoi(vecIndex->GetParameter("NumberOfThreads").c_str()), nullptr))
        return AnnIndex(0);

    return AnnIndex(vecIndex);
}

bool 
AnnIndex::Test()
{
    std::cout << " Test Called from python \n";
    return 1;
}