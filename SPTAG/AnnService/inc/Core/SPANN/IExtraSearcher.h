// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _SPTAG_SPANN_IEXTRASEARCHER_H_
#define _SPTAG_SPANN_IEXTRASEARCHER_H_

#include "Options.h"

#include "inc/Core/VectorIndex.h"
#include "inc/Helper/AsyncFileReader.h"

#include <memory>
#include <vector>
#include <chrono>
#include <atomic>

template <typename T>
class ConcurrentQueue
{
public:
    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            // std::cout << "Waitig Pop1\n";
            cond_.wait(mlock);
        }
        auto val = queue_.front();
        queue_.pop();
        consumed++;
        mlock.unlock();
        cond_.notify_one();
        return val;
    }

    bool pop(T &item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        if (consumed >= inverted_index_size)
        {
            // std::cout << "Consumed: " << consumed << " Inverted Index Size: " << inverted_index_size << "\n";
            return 0;
        }
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        item = queue_.front();
        queue_.pop();
        consumed++;
        mlock.unlock();
        cond_.notify_one();
        return 1;
    }

    bool PopFromMultiQueue(T &item, int index)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        if (consumed_vec[index] >= thread_queue_size[index])
            return 0;

        while (queue_vec[index].empty())
        {
            cond_.wait(mlock);
        }

        item = queue_vec[index].front();
        queue_vec[index].pop();
        consumed_vec[index]++;
        mlock.unlock();
        cond_.notify_one();
        return 1;
    }

    // void pushInMultiQueue(const T& item, int index){
    //     if(index >= queue_vec.size()) {
    //         std::cout << "Error: Index out of range\n";
    //         return;
    //     }

    //     if(!PushChecker()) {
    //         return;
    //     }

    //     char *temp;
    //     temp = new char[item.listInfo.listTotalBytes];
    //     memcpy(temp, item.fullData, item.listInfo.listTotalBytes);
    //     T newItem = item;
    //     newItem.fullData = temp;
    //     std::unique_lock<std::mutex> mlock(mutex_);
    //     queue_vec[index].push(newItem);
    //     mlock.unlock();
    //     cond_.notify_one();
    // }

    void pushInMultiQueue(const T &item, std::vector<int> &indexes)
    {

        if (!PushChecker())
        {
            return;
        }

        std::unique_lock<std::mutex> mlock(mutex_);
        for (auto index : indexes)
            queue_vec[index].push(item);

        mlock.unlock();
        cond_.notify_one();
    }

    void push(const T &item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        // std::cout << "Pushing Fun\n";
        if (!PushChecker())
        {
            return;
        }

        char *temp;
        temp = new char[item.listInfo.listTotalBytes];
        memcpy(temp, item.fullData, item.listInfo.listTotalBytes);
        T newItem = item;
        newItem.fullData = temp;
        queue_.push(newItem);

        // produced+=batch_read;
        produced++;
        mlock.unlock();
        cond_.notify_one();
    }

    bool PushChecker()
    {
        // std::unique_lock<std::mutex> mlock(mutex_);
        if (produced >= inverted_index_size)
        {
            return 0;
        }
        return 1;
    }

    int getProudced()
    {
        return produced;
    }

    void IncrementProduced()
    {
        produced++;
    }

    void setBatchRead(int batch_read)
    {
        this->batch_read = batch_read;
    }

    size_t size()
    {
        return produced;
    }

    void incrementThreadQueueSize(int index)
    {
        thread_queue_size[index]++;
    }

    void initThreads(int size)
    {
        queue_vec.resize(size);
        consumed_vec.resize(size);
        thread_queue_size.resize(size);
    }

    void setInvertedIndexSize(int size)
    {
        inverted_index_size = size;
    }

    ConcurrentQueue() = default;
    // ConcurrentQueue(const ConcurrentQueue&) = delete;            // disable copying
    ConcurrentQueue &operator=(const ConcurrentQueue &) = delete; // disable assignment
    ConcurrentQueue(int size) : inverted_index_size(size) {}
    ConcurrentQueue(int invertedIndexSize, int queueSize)
    {
        inverted_index_size = invertedIndexSize;
        queue_vec.resize(queueSize);

        // produced_vec.resize(queueSize);

        consumed_vec.resize(queueSize);
        thread_queue_size.resize(queueSize);
    }

private:
    std::queue<T> queue_;

    std::vector<std::queue<T>> queue_vec;
    // std::vector<int> produced_vec;
    std::vector<int> consumed_vec;
    std::vector<int> thread_queue_size;

    std::mutex mutex_;
    std::condition_variable cond_;
    // const static unsigned int BUFFER_SIZE = 100;
    int inverted_index_size;
    std::atomic<int> produced{0};
    int consumed = 0;
    int batch_read = 0;
};
namespace SPTAG
{
    namespace SPANN
    {
        struct QueueData;
        struct ListInfo
        {
            std::size_t listTotalBytes = 0;

            int listEleCount = 0;

            std::uint16_t listPageCount = 0;

            std::uint64_t listOffset = 0;

            std::uint16_t pageOffset = 0;
        };

        struct SearchStats
        {
            SearchStats()
                : m_check(0),
                  m_exCheck(0),
                  m_totalListElementsCount(0),
                  m_diskIOCount(0),
                  m_diskAccessCount(0),
                  m_totalSearchLatency(0),
                  m_totalLatency(0),
                  m_exLatency(0),
                  m_asyncLatency0(0),
                  m_asyncLatency1(0),
                  m_asyncLatency2(0),
                  m_queueLatency(0),
                  m_sleepLatency(0)
            {
            }

            int m_check;

            int m_exCheck;

            int m_totalListElementsCount;

            int m_diskIOCount;

            int m_diskAccessCount;

            double m_totalSearchLatency;

            double m_totalLatency;

            double m_exLatency;

            double m_asyncLatency0;

            double m_asyncLatency1;

            double m_asyncLatency2;

            double m_queueLatency;

            double m_sleepLatency;

            std::chrono::steady_clock::time_point m_searchRequestTime;

            int m_threadID;
        };

        template <typename T>
        class PageBuffer
        {
        public:
            PageBuffer()
                : m_pageBufferSize(0)
            {
            }

            void ReservePageBuffer(std::size_t p_size)
            {
                if (m_pageBufferSize < p_size)
                {
                    m_pageBufferSize = p_size;
                    m_pageBuffer.reset(static_cast<T *>(PAGE_ALLOC(sizeof(T) * m_pageBufferSize)), [=](T *ptr)
                                       { PAGE_FREE(ptr); });
                }
            }

            T *GetBuffer()
            {
                return m_pageBuffer.get();
            }

            std::size_t GetPageSize()
            {
                return m_pageBufferSize;
            }

        private:
            std::shared_ptr<T> m_pageBuffer;

            std::size_t m_pageBufferSize;
        };

        struct ExtraWorkSpace
        {
            ExtraWorkSpace() {}

            ~ExtraWorkSpace() {}

            ExtraWorkSpace(ExtraWorkSpace &other)
            {
                Initialize(other.m_deduper.MaxCheck(), other.m_deduper.HashTableExponent(), (int)other.m_pageBuffers.size(), (int)(other.m_pageBuffers[0].GetPageSize()), other.m_enableDataCompression);
                m_spaceID = g_spaceCount++;
            }

            void Initialize(int p_maxCheck, int p_hashExp, int p_internalResultNum, int p_maxPages, bool enableDataCompression)
            {
                m_postingIDs.reserve(p_internalResultNum);
                m_deduper.Init(p_maxCheck, p_hashExp);
                m_processIocp.reset(p_internalResultNum);
                m_pageBuffers.resize(p_internalResultNum);
                for (int pi = 0; pi < p_internalResultNum; pi++)
                {
                    m_pageBuffers[pi].ReservePageBuffer(p_maxPages);
                }
                m_diskRequests.resize(p_internalResultNum);
                m_enableDataCompression = enableDataCompression;
                if (enableDataCompression)
                {
                    m_decompressBuffer.ReservePageBuffer(p_maxPages);
                }
            }

            void Initialize(va_list &arg)
            {
                int maxCheck = va_arg(arg, int);
                int hashExp = va_arg(arg, int);
                int internalResultNum = va_arg(arg, int);
                int maxPages = va_arg(arg, int);
                bool enableDataCompression = bool(va_arg(arg, int));
                Initialize(maxCheck, hashExp, internalResultNum, maxPages, enableDataCompression);
            }

            static void Reset() { g_spaceCount = 0; }

            std::vector<int> m_postingIDs;

            COMMON::OptHashPosVector m_deduper;

            Helper::RequestQueue m_processIocp;

            std::vector<PageBuffer<std::uint8_t>> m_pageBuffers;

            bool m_enableDataCompression;
            PageBuffer<std::uint8_t> m_decompressBuffer;

            std::vector<Helper::AsyncReadRequest> m_diskRequests;

            int m_spaceID;

            static std::atomic_int g_spaceCount;
        };

        struct QueueData
        {
            ListInfo listInfo;
            int clusterID;
            std::shared_ptr<char> fullData;
            std::shared_ptr<ExtraWorkSpace> workSpace;
        };
        class IExtraSearcher
        {
        public:
            IExtraSearcher()
            {
            }

            virtual ~IExtraSearcher()
            {
            }

            virtual bool LoadIndex(Options &p_options) = 0;

            virtual void SearchIndex(ExtraWorkSpace *p_exWorkSpace,
                                     QueryResult &p_queryResults,
                                     std::shared_ptr<VectorIndex> p_index,
                                     SearchStats *p_stats,
                                     std::set<int> *truth = nullptr,
                                     std::map<int, std::set<int>> *found = nullptr) = 0;

            virtual void SearchInvertedIndex(ExtraWorkSpace *p_exWorkSpace, std::vector<QueryResult *> &queries,
                                             SearchStats *p_stats, std::shared_ptr<VectorIndex> p_index, QueueData queueData){};

            virtual void LoadFromDisk(std::shared_ptr<ExtraWorkSpace> p_exWorkSpace, std::vector<int> p_posting_ids,
                                      ConcurrentQueue<QueueData> &requests_data, std::map<int, std::vector<int>> &centroidThreadMap){};

            virtual bool BuildIndex(std::shared_ptr<Helper::VectorSetReader> &p_reader,
                                    std::shared_ptr<VectorIndex> p_index,
                                    Options &p_opt) = 0;

            virtual bool CheckValidPosting(SizeType postingID) = 0;
            virtual ListInfo GetListInfo(SizeType postingID){};
        };

    } // SPANN
} // SPTAG

#endif // _SPTAG_SPANN_IEXTRASEARCHER_H_