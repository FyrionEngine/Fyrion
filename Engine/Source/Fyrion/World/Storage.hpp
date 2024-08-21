#pragma once


#include "WorldTypes.hpp"
#include <Fyrion/Core/Array.hpp>
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion::World1
{
    template <usize PageSize>
    class BaseEntitySparseSet
    {
    public:
        using Iterator = Array<Entity>::Iterator;

        u64 Index(const Entity value) const
        {
            return sparse[Page(value)][Offset(value)];
        }

        usize Emplace(const Entity value)
        {
            usize index = count++;
            Assure(Page(value))[Offset(value)] = index;
            return index;
        }

        void Remove(const Entity value)
        {
            sparse[Page(value)][Offset(value)] = UINT64_MAX;
            count--;
        }

        bool Has(const Entity value) const
        {
            const auto curr = Page(value);
            return (curr < sparse.Size() && sparse[curr] && sparse[curr][Offset(value)] != UINT64_MAX);
        }

        usize Size() const
        {
            return count;
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
        }

        virtual ~BaseEntitySparseSet()
        {
            Clear();
        }

        usize IndexLast() const
        {
            return count - 1;
        }

    private:
        constexpr static usize Page(const u64 value)
        {
            return usize{value / PageSize};
        }

        constexpr static usize Offset(const u64 value)
        {
            return usize{value & (PageSize - 1)};
        }


        Entity* Assure(const usize pos)
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

        Array<Entity*> sparse;
        usize          count{};
        Allocator&     allocator{MemoryGlobals::GetDefaultAllocator()};
    };

    using EntitySparseSet = BaseEntitySparseSet<1024>;


    using EntityChunk = u8*;

    namespace Internal
    {
        FY_FINLINE usize& ChunkGetCount(EntityChunk chunk)
        {
            return *reinterpret_cast<usize*>(chunk + CountOffset);
        }

        FY_FINLINE Entity* ChunkGetEntities(EntityChunk chunk)
        {
            return reinterpret_cast<Entity*>(chunk + EntitiesOffset);
        }

        FY_FINLINE u8* ChunkGetData(EntityChunk chunk)
        {
            return chunk + DataOffset;
        }
    }

    class ComponentStorage
    {
    public:
        explicit ComponentStorage(TypeHandler* typeHandler) : typeHandler(typeHandler), typeSize(typeHandler->GetTypeInfo().size) {}

        ~ComponentStorage()
        {
            for (int i = 0; i < chunks.Size(); ++i)
            {
                DestroyChunk(i);
            }
        }

        VoidPtr Emplace(Entity entity, bool construct = true)
        {
            bool        newEntity = !sparse.Has(entity);
            u64         index = newEntity ? sparse.Emplace(entity) : sparse.Index(entity);
            usize       offset = Offset(index);
            EntityChunk chunk = GetChunk(Page(index));

            if (newEntity)
            {
                Internal::ChunkGetEntities(chunk)[offset] = entity;
                ++Internal::ChunkGetCount(chunk);
            }

            VoidPtr memory = &Internal::ChunkGetData(chunk)[offset * typeSize];

            if (construct)
            {
                typeHandler->Construct(memory);
            }
            return memory;
        }

        ConstPtr Get(Entity entity)
        {
            u64 index = sparse.Index(entity);
            return &Internal::ChunkGetData(GetChunk(Page(index)))[Offset(index) * typeSize];
        }

        VoidPtr GetMut(Entity entity)
        {
            u64 index = sparse.Index(entity);
            return &Internal::ChunkGetData(GetChunk(Page(index)))[Offset(index) * typeSize];
        }

        bool Has(Entity entity) const
        {
            return sparse.Has(entity);
        }

        void Remove(Entity entity)
        {
            usize       indexRemoved = sparse.Index(entity);
            usize       indexLast = sparse.IndexLast();
            EntityChunk lastChunk = GetChunk(Page(indexRemoved));

            if (indexRemoved != indexLast)
            {
                typeHandler->Move(&Internal::ChunkGetData(lastChunk)[Offset(indexLast)],
                                  &Internal::ChunkGetData(GetChunk(Page(indexRemoved)))[Offset(indexRemoved)]);
            }

            usize& count = Internal::ChunkGetCount(lastChunk);
            if (count - 1 == 0) DestroyChunk(Page(indexLast)); else count--;
            sparse.Remove(entity);
        }

        Span<EntityChunk> GetChunks() const
        {
            return chunks;
        }

    private:
        EntityChunk GetChunk(u64 page)
        {
            if (page >= chunks.Size())
            {
                chunks.Resize(page + 1);
            }

            if (!chunks[page])
            {
                chunks[page] = static_cast<EntityChunk>(MemoryGlobals::GetDefaultAllocator().MemAlloc(Internal::DataOffset + Internal::ChunkSize * typeSize, 1));
                Internal::ChunkGetCount(chunks[page]) = 0;
            }

            return chunks[page];
        }

        void DestroyChunk(u64 page)
        {
            if (EntityChunk chunk = chunks[page])
            {
                typeHandler->BatchDestructor(Internal::ChunkGetData(chunk), Internal::ChunkGetCount(chunk));
                MemoryGlobals::GetDefaultAllocator().MemFree(chunk);
                chunks[page] = nullptr;
            }
        }

        constexpr static usize Page(const u64 value)
        {
            return usize{value / Internal::ChunkSize};
        }

        constexpr static usize Offset(const u64 value)
        {
            return usize{value & Internal::ChunkSize - 1};
        }

        TypeHandler*       typeHandler;
        usize              typeSize;
        Array<EntityChunk> chunks;
        EntitySparseSet    sparse;
    };


    class EntityStorage final
    {
    public:

        EntityStorage() = default;
        FY_NO_COPY_CONSTRUCTOR(EntityStorage);

        ~EntityStorage()
        {
            for (EntityData* page : entities)
            {
                MemoryGlobals::GetDefaultAllocator().MemFree(page);
            }
        }

        Entity CreateEntity()
        {
            if (!freeEntities.Empty())
            {
                Entity entity = freeEntities.Back();
                freeEntities.PopBack();
                Get(entity).alive = true;
                return entity;
            }

            Entity entity = entityCount++;
            GetDataList(Page(entity))[Offset(entity)].alive = true;

            return entity;
        }

        void Destroy(Entity entity)
        {
            Get(entity).alive = false;
            freeEntities.EmplaceBack(entity);
        }

        EntityData& Get(Entity entity)
        {
            return entities[Page(entity)][Offset(entity)];
        }

        const EntityData* GetSafe(Entity entity) const
        {
            auto page = Page(entity);
            return page < entities.Size() ? &entities[page][Offset(entity)] : nullptr;
        }

        usize Count() const
        {
            return entityCount - freeEntities.Size() - 1;
        }

    private:
        constexpr static u16 ListSize = 11264;
        usize                entityCount = 1;

        Array<EntityData*> entities;
        Array<Entity>      freeEntities;    //TODO: improve freeEntities list


        constexpr static usize Page(const u64 value)
        {
            return usize{value / ListSize};
        }

        constexpr static usize Offset(const u64 value)
        {
            return usize{value & ListSize - 1};
        }

        EntityData* GetDataList(const usize page)
        {
            if (page >= entities.Size())
            {
                entities.Resize(page + 1);
            }

            if (!entities[page])
            {
                entities[page] = static_cast<EntityData*>(MemoryGlobals::GetDefaultAllocator().MemAlloc(ListSize * sizeof(EntityData), alignof(EntityData)));
            }

            return entities[page];
        }
    };
}
