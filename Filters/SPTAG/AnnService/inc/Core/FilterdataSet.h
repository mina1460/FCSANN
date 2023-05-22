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

    virtual ByteArray GetAllFilterdata() const = 0;

    virtual void Add(const ByteArray& data) = 0;
 
    virtual void AddBatch(FilterdataSet& data);

    virtual SizeType GetFilterNum () const = 0;

    virtual DimensionType GetFilterDim () const = 0;

    virtual SizeType Count() const = 0;

    virtual ErrorCode SaveToFile (const std::string& m_fileName) = 0;

    virtual ErrorCode LoadFromFile (const std::string& m_fileName) = 0;
    
};


class MemFilterdataSet : public FilterdataSet
{
public:

    MemFilterdataSet();
    
    MemFilterdataSet(ByteArray p_filterdata, SizeType p_countPerVector, DimensionType p_dim, SizeType p_count, SizeType filterSizePerVector);

    ~MemFilterdataSet();

    ByteArray GetFilterdata(SizeType p_vectorID) const;

    ByteArray GetAllFilterdata() const;

    void Add(const ByteArray& data);

    SizeType GetFilterNum () const;

    DimensionType GetFilterDim () const;

    SizeType Count() const;

    ErrorCode SaveToFile (const std::string& m_fileName);

    ErrorCode LoadFromFile (const std::string& m_fileName);

private:

    ByteArray m_filterdataHolder;       // The holder of all filterdata

    std::vector<std::uint8_t> m_newdata;

    DimensionType m_filtersDim;         // The dimension of each filterdata

    SizeType m_filterCountPerVector;    // The number of filters per vector

    SizeType m_count;                   // The number of vectors (data)

    size_t m_perFilterDataSize;
};


} // namespace SPTAG

#endif // _SPTAG_METADATASET_H_
