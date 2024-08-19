#pragma once
#include "Fyrion/Common.hpp"
#include <Fyrion/Core/Array.hpp>

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    using Entity = u64;

    // struct ComponentState
    // {
    //     u64 lastChange = 0;
    //     u64 lastCheck = 0;
    // };

    template <usize PageSize>
    class BaseEntitySparseSet
    {
    public:
        using Iterator = Array<u64>::Iterator;

        BaseEntitySparseSet() = default;

        u64 Index(const u64 value) const
        {
            return sparse[Page(value)][Offset(value)];
        }

        u64 Emplace(const u64 value)
        {
            auto index = packed.Size();
            Assure(Page(value))[Offset(value)] = index;
            packed.EmplaceBack(value);
            return index;
        }

        void Remove(const u64 value)
        {
            const auto curr = Page(value);
            const auto pos = Offset(value);
            packed[sparse[curr][pos]] = packed.Back();
            sparse[Page(packed.Back())][Offset(packed.Back())] = sparse[curr][pos];
            packed.PopBack();
            sparse[curr][pos] = UINT64_MAX;
        }

        bool Has(const u64 value) const
        {
            const auto curr = Page(value);
            return (curr < sparse.Size() && sparse[curr] && sparse[curr][Offset(value)] != UINT64_MAX);
        }

        usize Size() const
        {
            return packed.Size();
        }

        const Span<u64>& Data()
        {
            return packed;
        }

        void Clear()
        {
            for (auto it : sparse)
            {
                if (it)
                {
                    allocator.MemFree(it);
                }
            }
            sparse.Clear();
            packed.Clear();
        }

        virtual ~BaseEntitySparseSet()
        {
            Clear();
        }

        Span<u64> Packed()
        {
            return packed;
        }

    private:
        constexpr static auto Page(const u64 value)
        {
            return usize{value / PageSize};
        }

        constexpr static auto Offset(const u64 value)
        {
            return usize{value & (PageSize - 1)};
        }


        u64* Assure(const usize pos)
        {
            if (pos >= sparse.Size())
            {
                sparse.Resize(pos + 1);
            }
            if (!sparse[pos])
            {
                sparse[pos] = static_cast<u64*>(allocator.MemAlloc(PageSize * sizeof(u64), alignof(u64)));
                for (auto *first = sparse[pos], *last = first + PageSize; first != last; ++first)
                {
                    *first = UINT64_MAX;
                }
            }
            return sparse[pos];
        }

        Allocator&  allocator{MemoryGlobals::GetDefaultAllocator()};
        Array<u64*> sparse{};
        Array<u64>  packed{};
    };

    using EntitySparseSet = BaseEntitySparseSet<1024>;
    using ChunkStorage = u8*;

    class Storage
    {
    public:
        explicit Storage(TypeHandler* typeHandler) : typeHandler(typeHandler), typeSize(typeHandler->GetTypeInfo().size) {}

        VoidPtr Emplace(Entity entity)
        {
            u64 index = sparse.Emplace(entity);
            ChunkStorage chunk = AssureChunk(Page(index));
            VoidPtr memory = &chunk[Offset(index) * typeSize];
            typeHandler->Construct(memory);
            return memory;
        }

        VoidPtr Get(Entity entity)
        {
            u64 index = sparse.Index(entity);
            return &AssureChunk(Page(index))[Offset(index) * typeSize];
        }

    private:
        ChunkStorage AssureChunk(u64 page)
        {
            if (page >= chunks.Size())
            {
                chunks.Resize(page + 1);
            }

            if (!chunks[page])
            {
                chunks[page] = static_cast<ChunkStorage>(MemoryGlobals::GetDefaultAllocator().MemAlloc(typeSize * ChunkSize, typeHandler->GetTypeInfo().alignment));
            }
            return chunks[page];
        }


        constexpr static u16 ChunkSize = 1024;

        constexpr static auto Page(const u64 value)
        {
            return usize{value / ChunkSize};
        }

        constexpr static auto Offset(const u64 value)
        {
            return usize{value & ChunkSize - 1};
        }

        TypeHandler*         typeHandler;
        usize                typeSize;
        EntitySparseSet      sparse;
        Array<ChunkStorage> chunks;
    };
}
