// Sherif Gabr

#include "inc/Core/FilterdataSet.h"
// #include "FilterdataSet.h"

#include <string.h>
#include <shared_mutex>

using namespace SPTAG;

#include "inc/Helper/LockFree.h"

// typedef typename SPTAG::Helper::LockFree::LockFreeVector<std::uint64_t> FilterdataOffsets;


void
FilterdataSet::AddBatch(FilterdataSet& data)
{
    for (SizeType i = 0; i < data.Count(); i++)
    {
        Add(data.GetFilterdata(i));
    }
}

FilterdataSet::FilterdataSet()
{
}


FilterdataSet::~FilterdataSet()
{
}

MemFilterdataSet::MemFilterdataSet()
{
}

MemFilterdataSet::MemFilterdataSet(ByteArray p_filterdata, SizeType p_countPerVector, DimensionType p_dim, SizeType p_count, SizeType filterSizePerVector)
    : m_filterdataHolder(std::move(p_filterdata)),
      m_count(p_count), 
      m_filtersDim(p_dim),
      m_filterCountPerVector(p_countPerVector),
      m_perFilterDataSize(filterSizePerVector)
{
    // m_pOffsets.reset(new FilterdataOffsets, std::default_delete<FilterdataOffsets>());
    // auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    // m_offsets.reserve(p_count + 1, p_count + 1);
    // const std::uint64_t* newdata = reinterpret_cast<const std::uint64_t*>(p_offsets.Data());
    // m_offsets.assign(newdata, newdata + p_count + 1);
    // m_lock.reset(new std::shared_timed_mutex, std::default_delete<std::shared_timed_mutex>());
    LOG(Helper::LogLevel::LL_Info, "Constructed FilterdataSet\n");
}


MemFilterdataSet::~MemFilterdataSet()
{
}


ByteArray
MemFilterdataSet::GetFilterdata(SizeType p_vectorID) const
{   
    if (p_vectorID < 0 || p_vectorID >= m_count)
    {
        LOG(Helper::LogLevel::LL_Error, "Invalid vector ID: %d, with max ID: %d\n", p_vectorID, m_count);
        return ByteArray::c_empty;
    }

    // return reinterpret_cast<void*>(m_filterdataHolder.Data() + ((size_t)p_vectorID) * m_perFilterDataSize);
    // printf("%f\n", m_filterdataHolder[0]);
    return ByteArray(m_filterdataHolder.Data() + ((size_t)p_vectorID * m_perFilterDataSize), m_perFilterDataSize, false);
}

ByteArray 
MemFilterdataSet::GetAllFilterdata() const {
    return m_filterdataHolder;
}

SizeType
MemFilterdataSet::GetFilterNum() const
{
    return m_filterCountPerVector;
}

DimensionType
MemFilterdataSet::GetFilterDim() const
{
    return m_filtersDim;
}

SizeType
MemFilterdataSet::Count() const
{
    return m_count;
}

ErrorCode
MemFilterdataSet::SaveToFile(const std::string& m_fileName)
{
    printf("Saving Count: %d\n", m_count);
    printf("Saving FilterCountPerVector: %d\n", m_filterCountPerVector);
    printf("Saving FilterDim: %d\n", m_filtersDim);
    printf("Saving FilterdataHolder Length: %d\n", m_filterdataHolder.Length());

    std::ofstream out(m_fileName.c_str(), std::ios::binary);
    if (!out.is_open()) {
        LOG(Helper::LogLevel::LL_Error, "Failed to open file %s for writing!\n", m_fileName.c_str());
        return ErrorCode::FailedCreateFile;
    }
    size_t length = m_filterdataHolder.Length();

    out.write(reinterpret_cast<const char*>(&m_count), sizeof(SizeType));
    out.write(reinterpret_cast<const char*>(&m_filterCountPerVector), sizeof(SizeType));
    out.write(reinterpret_cast<const char*>(&m_filtersDim), sizeof(DimensionType));
    out.write(reinterpret_cast<const char*>(&length), sizeof(std::size_t));
    out.write(reinterpret_cast<const char*>(m_filterdataHolder.Data()), m_filterdataHolder.Length());
    out.close();

    for (int i = 0; i < 10; i++) {
        printf("Saved = %f\n", *reinterpret_cast<float*>(m_filterdataHolder.Data() + ((size_t)i * m_perFilterDataSize)));
    }
    
    // auto ptr = SPTAG::f_createIO();
    // if (ptr == nullptr || !ptr->Initialize(m_fileName.c_str(), std::ios::binary | std::ios::out)) return ErrorCode::FailedCreateFile;
    // size_t length = m_filterdataHolder.Length();
    // IOBINARY(ptr, WriteBinary, sizeof(SizeType), (char*)&m_count);
    // IOBINARY(ptr, WriteBinary, sizeof(SizeType), (char*)&m_filterCountPerVector);
    // IOBINARY(ptr, WriteBinary, sizeof(DimensionType), (char*)&m_filtersDim);
    // IOBINARY(ptr, WriteBinary, sizeof(std::size_t), (char*)&length);
    // IOBINARY(ptr, WriteBinary, m_filterdataHolder.Length(), (char*)m_filterdataHolder.Data());

    LOG(Helper::LogLevel::LL_Info, "Saved FilterdataSet to file %s\n", m_fileName.c_str());

    return ErrorCode::Success;
}

ErrorCode 
MemFilterdataSet::LoadFromFile(const std::string& m_fileName)
{
    std::ifstream in(m_fileName.c_str(), std::ios::binary);
    if (!in.is_open()) {
        LOG(Helper::LogLevel::LL_Error, "Failed to open file %s for reading!\n", m_fileName.c_str());
        return ErrorCode::FailedOpenFile;
    }
    // in.seekg(0, std::ios::end);
    // SizeType length = in.tellg();
    // in.seekg(0, std::ios::beg);
    std::size_t length;
    in.read(reinterpret_cast<char*>(&m_count), sizeof(SizeType));
    in.read(reinterpret_cast<char*>(&m_filterCountPerVector), sizeof(SizeType));
    in.read(reinterpret_cast<char*>(&m_filtersDim), sizeof(DimensionType));
    in.read(reinterpret_cast<char*>(&length), sizeof(std::size_t));

    printf("Count: %d\n", m_count);
    printf("FilterCountPerVector: %d\n", m_filterCountPerVector);
    printf("FilterDim: %d\n", m_filtersDim);
    printf("Length: %d\n", length);
    m_filterdataHolder = m_filterdataHolder.Alloc(length);
    
    in.read(reinterpret_cast<char*>(m_filterdataHolder.Data()), length);                 
    in.close();

    printf("FilterdataHolder Length: %d\n", m_filterdataHolder.Length());
    for (int i = 0; i < 10; i++) {
        printf("Loaded = %f\n", *reinterpret_cast<float*>(m_filterdataHolder.Data()));
    }

    // in.close();

    // auto ptr = SPTAG::f_createIO();
    // if (ptr == nullptr || !ptr->Initialize(m_fileName.c_str(), std::ios::binary | std::ios::in)) return ErrorCode::FailedCreateFile;

    // printf("Initialized loading filters\n");

    // IOBINARY(ptr, ReadBinary, sizeof(SizeType), (char*)&m_count);
    // IOBINARY(ptr, ReadBinary, sizeof(SizeType), (char*)&m_filterCountPerVector);
    // IOBINARY(ptr, ReadBinary, sizeof(DimensionType), (char*)&m_filtersDim);
    // IOBINARY(ptr, ReadBinary, length, (char*)m_filterdataHolder.Data());

    LOG(Helper::LogLevel::LL_Info, "Loaded FilterdataSet from file %s\n", m_fileName.c_str());

    return ErrorCode::Success;
}

void
MemFilterdataSet::Add(const ByteArray& data)
{
    // auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    // std::unique_lock<std::shared_timed_mutex> lock(*static_cast<std::shared_timed_mutex*>(m_lock.get()));
    // m_newdata.insert(m_newdata.end(), data.Data(), data.Data() + data.Length());
    // if (!m_offsets.push_back(m_offsets.back() + data.Length())) {
    //     LOG(Helper::LogLevel::LL_Error, "Insert FilterIndex error! DataCapacity overflow!\n");
    //     m_newdata.resize(m_newdata.size() - data.Length());
    // }
}
