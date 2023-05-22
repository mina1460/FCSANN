// Sherif Gabr

// #include "inc/Core/FilterdataSet.h"
#include "FilterdataSet.h"

#include <string.h>
#include <shared_mutex>

using namespace SPTAG;

#include "inc/Helper/LockFree.h"
typedef typename SPTAG::Helper::LockFree::LockFreeVector<std::uint64_t> FilterdataOffsets;

ErrorCode
FilterdataSet::RefineFilterdata(std::vector<SizeType>& indices, std::shared_ptr<FilterdataSet>& p_newMetadata,
    std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize) const
{
    p_newMetadata.reset(new MemFilterdataSet(p_blockSize, p_capacity, p_metaSize));
    for (SizeType& t : indices) {
        p_newMetadata->Add(GetFilterdata(t));
    }
    return ErrorCode::Success;
}

ErrorCode
FilterdataSet::RefineFilterdata(std::vector<SizeType>& indices, std::shared_ptr<Helper::DiskIO> p_metaOut, std::shared_ptr<Helper::DiskIO> p_metaIndexOut) const
{
    SizeType R = (SizeType)indices.size();
    IOBINARY(p_metaIndexOut, WriteBinary, sizeof(SizeType), (const char*)&R);
    std::uint64_t offset = 0;
    for (SizeType i = 0; i < R; i++) {
        IOBINARY(p_metaIndexOut, WriteBinary, sizeof(std::uint64_t), (const char*)&offset);
        ByteArray meta = GetFilterdata(indices[i]);
        offset += meta.Length();
    }
    IOBINARY(p_metaIndexOut, WriteBinary, sizeof(std::uint64_t), (const char*)&offset);

    for (SizeType i = 0; i < R; i++) {
        ByteArray meta = GetFilterdata(indices[i]);
        IOBINARY(p_metaOut, WriteBinary, sizeof(uint8_t) * meta.Length(), (const char*)meta.Data());
    }
    LOG(Helper::LogLevel::LL_Info, "Save MetaIndex(%d) Meta(%llu)\n", R, offset);
    return ErrorCode::Success;
}


ErrorCode 
FilterdataSet::RefineFilterdata(std::vector<SizeType>& indices, const std::string& p_metaFile, const std::string& p_metaindexFile) const
{
    {
        std::shared_ptr<Helper::DiskIO> ptrMeta = f_createIO(), ptrMetaIndex = f_createIO();
        if (ptrMeta == nullptr || ptrMetaIndex == nullptr || !ptrMeta->Initialize((p_metaFile + "_tmp").c_str(), std::ios::binary | std::ios::out) || !ptrMetaIndex->Initialize((p_metaindexFile + "_tmp").c_str(), std::ios::binary | std::ios::out))
        return ErrorCode::FailedCreateFile;

        ErrorCode ret = RefineFilterdata(indices, ptrMeta, ptrMetaIndex);
        if (ret != ErrorCode::Success) return ret;
    }

    if (fileexists(p_metaFile.c_str())) std::remove(p_metaFile.c_str());
    if (fileexists(p_metaindexFile.c_str())) std::remove(p_metaindexFile.c_str());
    std::rename((p_metaFile + "_tmp").c_str(), p_metaFile.c_str());
    std::rename((p_metaindexFile + "_tmp").c_str(), p_metaindexFile.c_str());
    return ErrorCode::Success;
}


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

bool FilterdataSet::GetFilterdataOffsets(const std::uint8_t* p_meta, const std::uint64_t p_metaLength, std::uint64_t* p_offsets, std::uint64_t p_offsetLength, char p_delimiter)
{
    std::uint64_t current = 0;
    p_offsets[current++] = 0;
    for (std::uint64_t i = 0; i < p_metaLength && current < p_offsetLength; i++) {
        if ((char)(p_meta[i]) == p_delimiter)
            p_offsets[current++] = (std::uint64_t)(i + 1);
    }
    if ((char)(p_meta[p_metaLength - 1]) != p_delimiter && current < p_offsetLength)
        p_offsets[current++] = p_metaLength;

    if (current < p_offsetLength) {
        LOG(Helper::LogLevel::LL_Error, "The metadata(%d) and vector(%d) numbers are not match! Check whether it is unicode encoding issue.\n", current-1, p_offsetLength-1);
        return false;
    }
    return true;
}

ErrorCode
MemFilterdataSet::Init(std::shared_ptr<Helper::DiskIO> p_metain, std::shared_ptr<Helper::DiskIO> p_metaindexin,
    std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize)
{
    IOBINARY(p_metaindexin, ReadBinary, sizeof(m_count), (char*)&m_count);
    m_pOffsets.reset(new FilterdataOffsets, std::default_delete<FilterdataOffsets>());
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    m_offsets.reserve(p_blockSize, p_capacity);
    {
        std::vector<std::uint64_t> tmp(m_count + 1, 0);
        IOBINARY(p_metaindexin, ReadBinary, sizeof(std::uint64_t) * (m_count + 1), (char*)tmp.data());
        m_offsets.assign(tmp.data(), tmp.data() + tmp.size());
    }
    m_filterdataHolder = ByteArray::Alloc(m_offsets[m_count]);
    IOBINARY(p_metain, ReadBinary, m_filterdataHolder.Length(), (char*)m_filterdataHolder.Data());

    m_newdata.reserve(p_blockSize * p_metaSize);
    m_lock.reset(new std::shared_timed_mutex, std::default_delete<std::shared_timed_mutex>());
    LOG(Helper::LogLevel::LL_Info, "Load MetaIndex(%d) Meta(%llu)\n", m_count, m_offsets[m_count]);
    return ErrorCode::Success;
}

MemFilterdataSet::MemFilterdataSet(ByteArray p_filterdata, SizeType p_countPerFilter, DimensionType p_dim, SizeType p_count)
    : m_count(p_count), m_filterdataHolder(std::move(p_filterdata))
{
    m_pOffsets.reset(new FilterdataOffsets, std::default_delete<FilterdataOffsets>());
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    m_offsets.reserve(p_count + 1, p_count + 1);
    const std::uint64_t* newdata = reinterpret_cast<const std::uint64_t*>(p_offsets.Data());
    m_offsets.assign(newdata, newdata + p_count + 1);
    m_lock.reset(new std::shared_timed_mutex, std::default_delete<std::shared_timed_mutex>());
}


MemFilterdataSet::~MemFilterdataSet()
{
}


ByteArray
MemFilterdataSet::GetFilterdata(SizeType p_vectorID) const
{
    // auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    // std::uint64_t startoff = m_offsets[p_vectorID];
    // std::uint64_t bytes = m_offsets[p_vectorID + 1] - startoff;
    // if (p_vectorID < m_count) {
    //     return ByteArray(m_filterdataHolder.Data() + startoff, bytes, false);
    // } else {
    //     std::shared_lock<std::shared_timed_mutex> lock(*static_cast<std::shared_timed_mutex*>(m_lock.get()));
    //     return ByteArray((std::uint8_t*)m_newdata.data() + startoff - m_offsets[m_count], bytes, false);
    // }
    if (p_vectorID < m_count){
        return ByteArray()
    }

    
}


ByteArray
MemFilterdataSet::GetFilterdataCopy(SizeType p_vectorID) const
{
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    std::uint64_t startoff = m_offsets[p_vectorID];
    std::uint64_t bytes = m_offsets[p_vectorID + 1] - startoff;
    if (p_vectorID < m_count) {
        ByteArray b = ByteArray::Alloc(bytes);
        memcpy(b.Data(), m_filterdataHolder.Data() + startoff, bytes);
        return b;
    } else {
        ByteArray b = ByteArray::Alloc(bytes);
        std::shared_lock<std::shared_timed_mutex> lock(*static_cast<std::shared_timed_mutex*>(m_lock.get()));
        memcpy(b.Data(), m_newdata.data() + (startoff - m_offsets[m_count]), bytes);
        return b;
    }
}


SizeType
MemFilterdataSet::Count() const
{
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    return static_cast<SizeType>(m_offsets.size() - 1);
}


bool
MemFilterdataSet::Available() const
{
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    return m_offsets.size() > 1;
}


std::pair<std::uint64_t, std::uint64_t>
MemFilterdataSet::BufferSize() const
{
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    auto n = m_offsets.size();
    return std::make_pair(m_offsets[n-1],
        sizeof(SizeType) + sizeof(std::uint64_t) * n);
}


void
MemFilterdataSet::Add(const ByteArray& data)
{
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    std::unique_lock<std::shared_timed_mutex> lock(*static_cast<std::shared_timed_mutex*>(m_lock.get()));
    m_newdata.insert(m_newdata.end(), data.Data(), data.Data() + data.Length());
    if (!m_offsets.push_back(m_offsets.back() + data.Length())) {
        LOG(Helper::LogLevel::LL_Error, "Insert FilterIndex error! DataCapacity overflow!\n");
        m_newdata.resize(m_newdata.size() - data.Length());
    }
}


ErrorCode
MemFilterdataSet::SaveFilterdata(std::shared_ptr<Helper::DiskIO> p_metaOut, std::shared_ptr<Helper::DiskIO> p_metaIndexOut)
{
    auto& m_offsets = *static_cast<FilterdataOffsets*>(m_pOffsets.get());
    SizeType count = Count();
    IOBINARY(p_metaIndexOut, WriteBinary, sizeof(SizeType), (const char*)&count);
    for (SizeType i = 0; i <= count; i++) {
        IOBINARY(p_metaIndexOut, WriteBinary, sizeof(std::uint64_t), (const char*)(&m_offsets[i]));
    }

    IOBINARY(p_metaOut, WriteBinary, m_filterdataHolder.Length(), reinterpret_cast<const char*>(m_filterdataHolder.Data()));
    if (m_newdata.size() > 0) {
        std::shared_lock<std::shared_timed_mutex> lock(*static_cast<std::shared_timed_mutex*>(m_lock.get()));
        IOBINARY(p_metaOut, WriteBinary, m_offsets[count] - m_offsets[m_count], (const char*)m_newdata.data());
    }
    LOG(Helper::LogLevel::LL_Info, "Save MetaIndex(%llu) Meta(%llu)\n", m_offsets.size() - 1, m_offsets.back());
    return ErrorCode::Success;
}



ErrorCode
MemFilterdataSet::SaveFilterdata(const std::string& p_metaFile, const std::string& p_metaindexFile)
{
    {
        std::shared_ptr<Helper::DiskIO> metaOut = f_createIO(), metaIndexOut = f_createIO();
        if (metaOut == nullptr || metaIndexOut == nullptr || !metaOut->Initialize((p_metaFile + "_tmp").c_str(), std::ios::binary | std::ios::out) || !metaIndexOut->Initialize((p_metaindexFile + "_tmp").c_str(), std::ios::binary | std::ios::out))
            return ErrorCode::FailedCreateFile;

        ErrorCode ret = SaveFilterdata(metaOut, metaIndexOut);
        if (ret != ErrorCode::Success) return ret;
    }
    if (fileexists(p_metaFile.c_str())) std::remove(p_metaFile.c_str());
    if (fileexists(p_metaindexFile.c_str())) std::remove(p_metaindexFile.c_str());
    std::rename((p_metaFile + "_tmp").c_str(), p_metaFile.c_str());
    std::rename((p_metaindexFile + "_tmp").c_str(), p_metaindexFile.c_str());
    return ErrorCode::Success;
}

