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
        AllocatorOptions_Verbose           = 1 << 0,
        AllocatorOptions_ShowStats         = 1 << 1,
        AllocatorOptions_ShowErrors        = 1 << 2,
        AllocatorOptions_DetectMemoryLeaks = 1 << 3,
        AllocatorOptions_CaptureStackTrace = 1 << 4
    };

    typedef u32 AllocatorOptions;

    struct FY_API Allocator
    {
        virtual ~Allocator() = default;

        virtual VoidPtr MemAlloc(usize bytes, usize alignment) = 0;
        virtual void    MemFree(VoidPtr ptr) = 0;
        virtual VoidPtr MemRealloc(VoidPtr ptr, usize newSize) = 0;

        template <typename Type, typename... Args>
        Type* Alloc(Args&&... args)
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

        template <typename Type>
        void DestroyAndFree(Type* type)
        {
            type->~Type();
            MemFree(type);
        }
    };

    struct FY_API GeneralPurposeAllocator : Allocator
    {
        VoidPtr MemAlloc(usize bytes, usize alignment) override;
        void    MemFree(VoidPtr ptr) override;
        VoidPtr MemRealloc(VoidPtr ptr, usize newSize) override;
    };

    namespace MemoryGlobals
    {
        FY_API Allocator& GetDefaultAllocator();
        FY_API void       SetOptions(AllocatorOptions options);
        FY_API HeapStats  GetHeapStats();
    }
}
