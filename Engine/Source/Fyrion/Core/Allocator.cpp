
#include "Allocator.hpp"
#include <cstdlib>
#include <mimalloc.h>
#include "mimalloc/types.h"

namespace Fyrion
{
    namespace
    {
        HeapAllocator defaultAllocator{};
    }

    VoidPtr HeapAllocator::MemAlloc(usize bytes, usize alignment)
    {
        return mi_malloc_aligned(bytes, alignment);
    }

    void HeapAllocator::MemFree(VoidPtr ptr)
    {
        mi_free(ptr);
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
    }

    Allocator& MemoryGlobals::GetDefaultAllocator()
    {
        return defaultAllocator;
    }

    HeapStats MemoryGlobals::GetHeapStats()
    {
        mi_heap_t* heap = mi_heap_get_default();
        return HeapStats{
            .totalAllocated = heap->tld->stats.normal.allocated + heap->tld->stats.large.allocated  + heap->tld->stats.huge.allocated,
            .totalFreed = heap->tld->stats.normal.freed + heap->tld->stats.large.freed  + heap->tld->stats.huge.freed,
        };
    }
}