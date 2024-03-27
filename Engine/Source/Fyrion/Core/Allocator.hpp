#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct FY_API Allocator
    {
        virtual VoidPtr     MemAlloc(usize bytes, usize alignment)   = 0;
        virtual void        MemFree(VoidPtr ptr)    = 0;
    };

    struct FY_API HeapAllocator : Allocator
    {
        VoidPtr     MemAlloc(usize bytes, usize alignment) override;
        void        MemFree(VoidPtr ptr) override;
    };


    namespace MemoryGlobals
    {
        FY_API Allocator& GetDefaultAllocator();
    }
}