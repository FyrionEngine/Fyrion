#pragma once

#include "Fyrion/Common.hpp"
#include "Traits.hpp"

namespace Fyrion
{

    struct HeapStats
    {
        i64 totalAllocated{};
        i64 totalFreed{};
    };

    enum AllocatorOptions_
    {
        AllocatorOptions_Verbose = 1 << 0,
        AllocatorOptions_ShowStats = 1 << 1,
        AllocatorOptions_ShowErrors = 1 << 2,
        AllocatorOptions_CaptureTrace = 1 << 3,
    };

    typedef u32 AllocatorOptions;

    struct FY_API Allocator
    {
        virtual VoidPtr     MemAlloc(usize bytes, usize alignment)   = 0;
        virtual void        MemFree(VoidPtr ptr)    = 0;

        template<typename Type, typename ...Args>
        Type* Alloc(Args&& ...args)
        {
            auto ptr = static_cast<Type*>(MemAlloc(sizeof(Type), alignof(Type)));
            if constexpr (Traits::IsAggregate<Type>)
            {
                new(PlaceHolder(), ptr) Type{Traits::Forward<Args>(args)...};
            }
            else
            {
                new(PlaceHolder(), ptr) Type(Traits::Forward<Args>(args)...);
            }
            return ptr;
        }

        template<typename Type>
        void DestroyAndFree(Type * type)
        {
            type->~Type();
            MemFree(type);
        }
    };

    struct FY_API HeapAllocator : Allocator
    {
        VoidPtr     MemAlloc(usize bytes, usize alignment) override;
        void        MemFree(VoidPtr ptr) override;
    };

    namespace MemoryGlobals
    {
        FY_API Allocator& GetDefaultAllocator();
        FY_API void       SetOptions(AllocatorOptions options);
        FY_API HeapStats  GetHeapStats();
    }
}