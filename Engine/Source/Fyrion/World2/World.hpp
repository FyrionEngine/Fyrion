#pragma once


#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"


namespace Fyrion::World2
{
    using Entity = u64;
    constexpr u8            ArchetypeMaxComponents = 128;
    constexpr u16           ChunkComponentSize = 16 * 1024;
    constexpr static Entity NullEntity = 0;

    struct ComponentState
    {
        u64 lastChange = 0;
        u64 lastCheck = 0;
    };

    struct ArchetypeType
    {
        TypeID       typeId;
        TypeHandler* typeHandler;
        usize        typeSize;
        usize        dataOffset;
        usize        stateOffset;
    };

    using ArchetypeChunk = u8*;

    struct Archetype
    {
        u32                   id;
        u64                   hash;
        usize                 maxEntityChunkCount;
        usize                 chunkTotalAllocSize;
        usize                 chunkDataSize;
        usize                 entityArrayOffset;
        usize                 entityCountOffset;
        usize                 chunkStateOffset;
        Array<ArchetypeType>  types;
        HashMap<TypeID, u16>  typeIndex;
        Array<ArchetypeChunk> chunks;
    };

    namespace Internal
    {
        FY_FINLINE usize& GetEntityCount(Archetype* archetype, const ArchetypeChunk& chunk)
        {
            return *reinterpret_cast<usize*>(&chunk[archetype->entityCountOffset]);
        }

        FY_FINLINE Entity& GetEntity(Archetype* archetype, const ArchetypeChunk& chunk, usize index)
        {
            return *reinterpret_cast<Entity*>(&chunk[archetype->entityArrayOffset + index * sizeof(Entity)]);
        }

        FY_FINLINE usize AddEntityChunk(Archetype* archetype, const ArchetypeChunk& chunk, Entity entity)
        {
            usize& count = GetEntityCount(archetype, chunk);
            usize index = ++count;
            GetEntity(archetype, chunk, index) = entity;
            return index;
        }

        FY_FINLINE VoidPtr GetChunkComponentData(const ArchetypeType& archetypeType, const ArchetypeChunk& chunk, usize index)
        {
            return &chunk[archetypeType.dataOffset + (index * archetypeType.typeSize)];
        }
    }

    using ArchetypeHashMap = HashMap<usize, Array<SharedPtr<Archetype>>>;
    using ArchetypeChunk = u8*;

    struct EntityData
    {
        Archetype*     archetype = nullptr;
        ArchetypeChunk chunk = nullptr;
        u32            chunkIndex = 0;
        bool           alive = true;
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

            new(&GetDataList(Page(entity))[Offset(entity)]) EntityData{
                .archetype = nullptr,
                .chunk = nullptr,
                .chunkIndex = 0,
                .alive = true
            };

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

        EntityData* GetSafe(Entity entity)
        {
            auto page = Page(entity);
            return page < entities.Size() ? &entities[page][Offset(entity)] : nullptr;
        }

        EntityData& FindOrCreate(Entity entity)
        {
            if (EntityData* data = GetSafe(entity))
            {
                return *data;
            }
            return Get(CreateEntity());
        }


        usize Count() const
        {
            return entityCount - freeEntities.Size() - 1;
        }

    private:
        constexpr static u16 ListSize = 11264;
        usize                entityCount = 1;

        Array<EntityData*> entities;
        Array<Entity>      freeEntities; //TODO: improve freeEntities list


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


    class FY_API World
    {
    public:
        World()
        {
            rootArchetype = CreateArchetype(0, nullptr, 0);
        }

        void Add(Entity entity, TypeID* types, VoidPtr* components, i16 size)
        {
            if (EntityData* entityData = entities.GetSafe(entity))
            {
                if (entityData->chunk == nullptr || entityData->archetype == nullptr)
                {
                    entityData->archetype = FindOrCreateArchetype(types, size);
                    entityData->chunk = FindOrCreateChunk(entityData->archetype);
                    entityData->chunkIndex = Internal::AddEntityChunk(entityData->archetype, entityData->chunk, entity);
                }
                else
                {
                    //TODO move
                }

                for (int i = 0; i < size; ++i)
                {
                    const TypeID typeId = types[i];
                    usize typeIndex = entityData->archetype->typeIndex.Find(typeId)->second;
                    ArchetypeType& type = entityData->archetype->types[typeIndex];

                    // ComponentState& state = FY_CHUNK_COMPONENT_STATE(type, entityContainer.chunk, entityContainer.chunkIndex);
                    // ComponentState& chunkState = FY_CHUNK_STATE(typeIndex, entityContainer.archetype, entityContainer.chunk);
                    //
                    // state.lastChange = m_currentFrame;
                    // state.lastCheck = 0;
                    // chunkState.lastChange = m_currentFrame;

                    VoidPtr data = Internal::GetChunkComponentData(type, entityData->chunk, entityData->chunkIndex);

                    if (type.typeHandler->GetTypeInfo().isTriviallyCopyable)
                    {
                        if (components[i])
                        {
                            MemCopy(data, components[i], type.typeHandler->GetTypeInfo().size);
                        }
                        else
                        {
                            MemSet(data, 0, type.typeHandler->GetTypeInfo().size);
                        }
                    }
                    else
                    {
                        if (components[i])
                        {
                            type.typeHandler->Copy(components[i], data);
                        }
                        else
                        {
                            type.typeHandler->Construct(data);
                        }
                    }



                    //type.sparse->Emplace(entity, data);
                }

            }
        }

        FY_FINLINE Entity SpawnWithComponents(Entity parent, TypeID* types, VoidPtr* components, i16 size)
        {
            Entity entity = entities.CreateEntity();
            Add(entity, types, components, size);
            return entity;
        }

        template <typename... Types>
        FY_FINLINE Entity Spawn(Types&&... types)
        {
            FixedArray<TypeID, sizeof...(Types)>  ids{GetTypeID<Types>()...};
            FixedArray<VoidPtr, sizeof...(Types)> components{&types...};
            return SpawnWithComponents(NullEntity, ids.begin(), components.begin(), sizeof...(Types));
        }

        void Remove(Entity entity, TypeID* types, usize size)
        {

        }

        void Destroy(Entity entity)
        {

        }

    private:
        EntityStorage    entities;
        ArchetypeHashMap archetypes;
        Archetype*       rootArchetype = nullptr;


        ArchetypeChunk FindOrCreateChunk(Archetype* archetype)
        {
            ArchetypeChunk activeChunk = !archetype->chunks.Empty() ? archetype->chunks.Back() : nullptr;

            if (activeChunk)
            {
                if (usize count = Internal::GetEntityCount(archetype, activeChunk); count < archetype->maxEntityChunkCount)
                {
                    return activeChunk;
                }

            }

            ArchetypeChunk chunk = static_cast<ArchetypeChunk>(MemoryGlobals::GetDefaultAllocator().MemAlloc(archetype->chunkTotalAllocSize, 1));
            MemSet(chunk, 0, archetype->chunkDataSize);
            archetype->chunks.EmplaceBack(chunk);
            return chunk;
        }

        Archetype* FindOrCreateArchetype(TypeID* ids, u16 size)
        {
            usize hash = Sum(ids, ids + size);
            if (auto it = archetypes.Find(hash))
            {
                if (it->second.Size() == 1)
                {
                    return it->second[0].Get();
                }
                //TODO
            }

            return CreateArchetype(hash, ids, size);
        }

        Archetype* CreateArchetype(usize hash, TypeID* ids, u16 size)
        {
            TypeID sortedIds[ArchetypeMaxComponents];
            MemCopy(sortedIds, ids, sizeof(TypeID) * size);


            Sort(sortedIds, sortedIds + size, [](TypeID left, TypeID right)
            {
                return left > right;
            });

            auto it = archetypes.Find(hash);
            if (it == archetypes.end())
            {
                it = archetypes.Emplace(hash, {}).first;
            }

            Archetype* archetype = it->second.EmplaceBack(MakeShared<Archetype>()).Get();
            archetype->hash = hash;
            archetype->id = 0; //world.archetypes.count();
            archetype->chunkTotalAllocSize = 0;
            archetype->entityArrayOffset = 0;
            archetype->types = {size};

            if (size > 0)
            {
                u32 stride = 0;
                for (u16 t = 0; t < size; ++t)
                {
                    if (TypeHandler* typeHandler = Registry::FindTypeById(sortedIds[t]))
                    {
                        archetype->typeIndex.Emplace(sortedIds[t], static_cast<usize>(t));

                        archetype->types[t] = ArchetypeType{
                            .typeId = sortedIds[t],
                            .typeHandler = typeHandler,
                            .typeSize = typeHandler->GetTypeInfo().size,
                        };

                        stride += typeHandler->GetTypeInfo().size;
                        continue;
                    }
                    FY_ASSERT(false, "type not found");
                }

                archetype->maxEntityChunkCount = Max(ChunkComponentSize / stride, 1u);
                archetype->chunkTotalAllocSize = archetype->maxEntityChunkCount * sizeof(Entity);
                archetype->entityCountOffset = archetype->chunkTotalAllocSize;
                archetype->chunkTotalAllocSize += sizeof(u32);
                archetype->chunkStateOffset = archetype->chunkTotalAllocSize;
                archetype->chunkTotalAllocSize += size * sizeof(ComponentState);
                archetype->chunkDataSize = archetype->chunkTotalAllocSize;

                for (u16 t = 0; t < size; ++t)
                {
                    archetype->types[t].dataOffset = archetype->chunkTotalAllocSize;
                    archetype->chunkTotalAllocSize += archetype->maxEntityChunkCount * archetype->types[t].typeSize;
                    archetype->types[t].stateOffset = archetype->chunkTotalAllocSize;
                    archetype->chunkTotalAllocSize += archetype->maxEntityChunkCount * sizeof(ComponentState);
                }

            }
            else
            {
                archetype->maxEntityChunkCount = ChunkComponentSize / sizeof(Entity);
                archetype->chunkTotalAllocSize = archetype->maxEntityChunkCount * sizeof(Entity);
                archetype->entityCountOffset = archetype->chunkTotalAllocSize;
                archetype->chunkTotalAllocSize += sizeof(u32);
            }

            return archetype;
        }
    };
}
