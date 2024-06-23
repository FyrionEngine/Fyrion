#include "Allocator.hpp"
#include <mimalloc.h>
#include "mimalloc/types.h"

#include <cpptrace/cpptrace.hpp>
#include <unordered_map>
#include <mutex>

namespace Fyrion
{
    namespace
    {
        void OnExit();

        bool Init()
        {
            atexit(OnExit);
            return true;
        }

        bool                                            captureTrace = false;
        bool                                            detectLeaks = false;
        std::unordered_map<usize, cpptrace::stacktrace> traces{};
        std::mutex                                      traceMutex{};
        bool                                            init = Init();


        void OnExit()
        {
            if (captureTrace && !traces.empty())
            {
                traceMutex.lock();
                for (auto& it : traces)
                {
                    it.second.print();
                }
                traceMutex.unlock();
                FY_ASSERT(false, "leak detected");
            }

            if (detectLeaks)
            {
                HeapStats heapStats = MemoryGlobals::GetHeapStats();
                if (heapStats.totalAllocated != heapStats.totalFreed)
                {
                    FY_ASSERT(false, "leak detected");
                }
            }
        }
    }

    VoidPtr GeneralPurposeAllocator::MemAlloc(usize bytes, usize alignment)
    {
        VoidPtr ptr = mi_malloc_aligned(bytes, alignment);

        if (captureTrace)
        {
            usize ptrAddress = reinterpret_cast<usize>(ptr);
            auto  trace = cpptrace::generate_trace(1);
            traceMutex.lock();
            traces.insert(std::make_pair(ptrAddress, std::move(trace)));
            traceMutex.unlock();
        }
        return ptr;
    }

    void GeneralPurposeAllocator::MemFree(VoidPtr ptr)
    {
        if (captureTrace)
        {
            traceMutex.lock();
            traces.erase(reinterpret_cast<usize>(ptr));
            traceMutex.unlock();
        }

        mi_free(ptr);
    }

    VoidPtr GeneralPurposeAllocator::MemRealloc(VoidPtr ptr, usize newSize)
    {
        VoidPtr newPtr = mi_realloc(ptr, newSize);
        if (captureTrace)
        {
            usize ptrAddress = reinterpret_cast<usize>(newPtr);
            auto  trace = cpptrace::generate_trace(1);
            traceMutex.lock();
            traces.insert(std::make_pair(ptrAddress, std::move(trace)));
            traceMutex.unlock();
        }
        return newPtr;
    }

    void MemoryGlobals::SetOptions(AllocatorOptions options)
    {
        if (options & AllocatorOptions_ShowStats)
        {
            mi_option_set(mi_option_show_stats, true);
        }

        if (options & AllocatorOptions_Verbose)
        {
            mi_option_set(mi_option_verbose, true);
        }

        if (options & AllocatorOptions_ShowErrors)
        {
            mi_option_set(mi_option_show_errors, true);
        }

        if (options & AllocatorOptions_DetectMemoryLeaks)
        {
            detectLeaks = true;

            if (options & AllocatorOptions_CaptureStackTrace)
            {
                captureTrace = true;
            }
        }


    }

    Allocator& MemoryGlobals::GetDefaultAllocator()
    {
        static GeneralPurposeAllocator defaultAllocator{};
        return defaultAllocator;
    }

    HeapStats MemoryGlobals::GetHeapStats()
    {
        mi_heap_t* heap = mi_heap_get_default();
        return HeapStats{
            .totalAllocated = heap->tld->stats.normal.allocated + heap->tld->stats.large.allocated + heap->tld->stats.huge.allocated,
            .totalFreed = heap->tld->stats.normal.freed + heap->tld->stats.large.freed + heap->tld->stats.huge.freed,
        };
    }
}
