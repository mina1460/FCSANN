// Sherif Gabr

#ifndef _SPTAG_FILTERDATASET_H_
#define _SPTAG_FILTERDATASET_H_

#include "CommonDataStructure.h"

namespace SPTAG
{

class FilterdataSet
{
public:
    FilterdataSet();

    virtual ~FilterdataSet();

    virtual ByteArray GetFilterdata(SizeType p_vectorID) const = 0;

    virtual ByteArray GetFilterdataCopy(SizeType p_vectorID) const = 0;

    virtual SizeType Count() const = 0;

    virtual bool Available() const = 0;

    virtual std::pair<std::uint64_t, std::uint64_t> BufferSize() const = 0;

    virtual void Add(const ByteArray& data) = 0;

    virtual ErrorCode SaveFilterdata(std::shared_ptr<Helper::DiskIO> p_metaOut, std::shared_ptr<Helper::DiskIO> p_metaIndexOut) = 0;

    virtual ErrorCode SaveFilterdata(const std::string& p_metaFile, const std::string& p_metaindexFile) = 0;
 
    virtual void AddBatch(FilterdataSet& data);
    
    virtual ErrorCode RefineFilterdata(std::vector<SizeType>& indices, std::shared_ptr<FilterdataSet>& p_newMetadata, std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize) const;

    virtual ErrorCode RefineFilterdata(std::vector<SizeType>& indices, std::shared_ptr<Helper::DiskIO> p_metaOut, std::shared_ptr<Helper::DiskIO> p_metaIndexOut) const;

    virtual ErrorCode RefineFilterdata(std::vector<SizeType>& indices, const std::string& p_metaFile, const std::string& p_metaindexFile) const;

    static bool GetFilterdataOffsets(const std::uint8_t* p_meta, const std::uint64_t p_metaLength, std::uint64_t* p_offsets, std::uint64_t p_offsetLength, char p_delimiter = '\n');
};


class MemFilterdataSet : public FilterdataSet
{
public:
    MemFilterdataSet(std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize);

    MemFilterdataSet(ByteArray p_metadata, ByteArray p_offsets, SizeType p_count);

    MemFilterdataSet(ByteArray p_metadata, ByteArray p_offsets, SizeType p_count, 
        std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize);

    MemFilterdataSet(const std::string& p_metafile, const std::string& p_metaindexfile, 
        std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize);

    MemFilterdataSet(std::shared_ptr<Helper::DiskIO> p_metain, std::shared_ptr<Helper::DiskIO> p_metaindexin, 
        std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize);

    ~MemFilterdataSet();

    ByteArray GetFilterdata(SizeType p_vectorID) const;
    
    ByteArray GetFilterdataCopy(SizeType p_vectorID) const;

    SizeType Count() const;

    bool Available() const;

    std::pair<std::uint64_t, std::uint64_t> BufferSize() const;

    void Add(const ByteArray& data);

    ErrorCode SaveFilterdata(std::shared_ptr<Helper::DiskIO> p_metaOut, std::shared_ptr<Helper::DiskIO> p_metaIndexOut);

    ErrorCode SaveFilterdata(const std::string& p_metaFile, const std::string& p_metaindexFile);

private:
    ErrorCode Init(std::shared_ptr<Helper::DiskIO> p_metain, std::shared_ptr<Helper::DiskIO> p_metaindexin,
        std::uint64_t p_blockSize, std::uint64_t p_capacity, std::uint64_t p_metaSize);

    std::shared_ptr<void> m_lock;

    std::shared_ptr<void> m_pOffsets;

    SizeType m_count;

    ByteArray m_filterdataHolder;

    std::vector<std::uint8_t> m_newdata;
};


} // namespace SPTAG

#endif // _SPTAG_METADATASET_H_
