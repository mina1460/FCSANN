// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/Core/VectorSet.h"
#include "inc/Core/Common/CommonUtils.h"

using namespace SPTAG;

VectorSet::VectorSet()
{
}


VectorSet::~VectorSet()
{
}


BasicVectorSet::BasicVectorSet(const ByteArray& p_bytesArray,
                               VectorValueType p_valueType,
                               DimensionType p_dimension,
                               SizeType p_vectorCount)
    : m_data(p_bytesArray),
      m_valueType(p_valueType),
      m_dimension(p_dimension),
      m_vectorCount(p_vectorCount),
      m_perVectorDataSize(static_cast<SizeType>(p_dimension * GetValueTypeSize(p_valueType)))
{
}

BasicVectorSet::BasicVectorSet(const ByteArray& p_bytesArray,
                VectorValueType p_valueType,
                DimensionType p_dimension,
                SizeType p_vectorCount,
                const ByteArray& p_filtersArray,
                DimensionType p_filtersDimension,
                SizeType p_numberOfFilters)
    : m_data(p_bytesArray),
      m_valueType(p_valueType),
      m_dimension(p_dimension),
      m_vectorCount(p_vectorCount),
      m_perVectorDataSize(static_cast<SizeType>(p_dimension * GetValueTypeSize(p_valueType))),
      m_filters(p_filtersArray),
      m_filtersDim(p_filtersDimension),
      m_filtersCount(p_numberOfFilters),
      m_perFilterDataSize(static_cast<SizeType>(p_filtersDimension * p_numberOfFilters * GetValueTypeSize(p_valueType)))
{
}


BasicVectorSet::~BasicVectorSet()
{
}


VectorValueType
BasicVectorSet::GetValueType() const
{
    return m_valueType;
}


void*
BasicVectorSet::GetVector(SizeType p_vectorID) const
{
    if (p_vectorID < 0 || p_vectorID >= m_vectorCount)
    {
        return nullptr;
    }

    return reinterpret_cast<void*>(m_data.Data() + ((size_t)p_vectorID) * m_perVectorDataSize);
}

void*
BasicVectorSet::GetFilters(SizeType p_vectorID) const 
{
    if (p_vectorID < 0 || p_vectorID >= m_vectorCount)
    {
        return nullptr;
    }

    return reinterpret_cast<void*>(m_filters.Data() + ((size_t)p_vectorID) * m_perFilterDataSize);
}

void*
BasicVectorSet::GetData() const
{
    return reinterpret_cast<void*>(m_data.Data());
}

void*
BasicVectorSet::GetFiltersData() const
{
    return reinterpret_cast<void*>(m_filters.Data());
}

DimensionType
BasicVectorSet::Dimension() const
{
    return m_dimension;
}

DimensionType
BasicVectorSet::FiltersDimension() const
{
    return m_filtersDim;
}

SizeType
BasicVectorSet::Count() const
{
    return m_vectorCount;
}

SizeType
BasicVectorSet::FiltersCount() const 
{
    return m_filtersCount;
}


bool
BasicVectorSet::Available() const
{
    return m_data.Data() != nullptr;
}


ErrorCode 
BasicVectorSet::Save(const std::string& p_vectorFile) const
{
    auto fp = SPTAG::f_createIO();
    if (fp == nullptr || !fp->Initialize(p_vectorFile.c_str(), std::ios::binary | std::ios::out)) return ErrorCode::FailedOpenFile;

    IOBINARY(fp, WriteBinary, sizeof(SizeType), (char*)&m_vectorCount);
    IOBINARY(fp, WriteBinary, sizeof(DimensionType), (char*)&m_dimension);
    IOBINARY(fp, WriteBinary, m_data.Length(), (char*)m_data.Data());
    return ErrorCode::Success;
}

ErrorCode BasicVectorSet::AppendSave(const std::string& p_vectorFile) const
{
    auto append = fileexists(p_vectorFile.c_str());
    
    SizeType count;
    SizeType dim;

    // Get count based on already written results
    if (append)
    {
        auto fp_read = SPTAG::f_createIO();
        if (fp_read == nullptr || !fp_read->Initialize(p_vectorFile.c_str(), std::ios::binary | std::ios::in)) return ErrorCode::FailedOpenFile;
        IOBINARY(fp_read, ReadBinary, sizeof(SizeType), (char*)&count);
        IOBINARY(fp_read, ReadBinary, sizeof(DimensionType), (char*)&dim);
        if (dim != m_dimension) { return ErrorCode::DimensionSizeMismatch; }
        count += m_vectorCount;
    }
    else
    {
        count = m_vectorCount;
        dim = m_dimension;
    }

    // Update count header
    {
        auto fp_write = SPTAG::f_createIO();
        if (fp_write == nullptr || !fp_write->Initialize(p_vectorFile.c_str(), std::ios::binary | std::ios::out | (append ? std::ios::in : 0))) return ErrorCode::FailedOpenFile;
        IOBINARY(fp_write, WriteBinary, sizeof(SizeType), (char*)&count);
        IOBINARY(fp_write, WriteBinary, sizeof(DimensionType), (char*)&dim);
    }


    // Write new vectors to end of file
    {
        auto fp_append = SPTAG::f_createIO();
        if (fp_append == nullptr || !fp_append->Initialize(p_vectorFile.c_str(), std::ios::binary | std::ios::out | std::ios::app)) return ErrorCode::FailedOpenFile;
        IOBINARY(fp_append, WriteBinary, m_data.Length(), (char*)m_data.Data());
    }

    return ErrorCode::Success;
}

SizeType BasicVectorSet::PerVectorDataSize() const 
{
    return (SizeType)m_perVectorDataSize;
}


void
BasicVectorSet::Normalize(int p_threads) 
{
    switch (m_valueType)
    {
#define DefineVectorValueType(Name, Type) \
case SPTAG::VectorValueType::Name: \
SPTAG::COMMON::Utils::BatchNormalize<Type>(reinterpret_cast<Type *>(m_data.Data()), m_vectorCount, m_dimension, SPTAG::COMMON::Utils::GetBase<Type>(), p_threads); \
break; \

#include "inc/Core/DefinitionList.h"
#undef DefineVectorValueType
    default:
        break;
    }
}

void
BasicVectorSet::NormalizeFilters(int p_threads) 
{
    switch (m_valueType)
    {
#define DefineVectorValueType(Name, Type) \
case SPTAG::VectorValueType::Name: \
SPTAG::COMMON::Utils::BatchNormalize<Type>(reinterpret_cast<Type *>(m_filters.Data()), m_vectorCount*m_filtersCount*m_filtersDim, m_filtersDim, SPTAG::COMMON::Utils::GetBase<Type>(), p_threads); \
break; \

#include "inc/Core/DefinitionList.h"
#undef DefineVectorValueType
    default:
        break;
    }
}