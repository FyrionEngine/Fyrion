
#include "Allocator.hpp"
#include <cstdlib>

namespace Fyrion
{
    namespace
    {
        HeapAllocator defaultAllocator{};
    }

    VoidPtr HeapAllocator::MemAlloc(usize bytes, usize alignment)
    {
        return malloc(bytes);
    }

    void HeapAllocator::MemFree(VoidPtr ptr)
    {
        free(ptr);
    }

    Allocator& MemoryGlobals::GetDefaultAllocator()
    {
        return defaultAllocator;
    }
}