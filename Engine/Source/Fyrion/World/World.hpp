#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class World;
    using Entity = u64;
    constexpr u8            ArchetypeMaxComponents = 128;
    constexpr u16           ChunkComponentSize = 16 * 1024;
    constexpr static Entity NullEntity = 0;

    template<typename ...Types>
    class Query;

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
        bool         isTriviallyCopyable;
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
        FY_FINLINE u32& GetEntityCount(Archetype* archetype, const ArchetypeChunk& chunk)
        {
            return *reinterpret_cast<u32*>(&chunk[archetype->entityCountOffset]);
        }

        FY_FINLINE Entity& GetEntity(Archetype* archetype, const ArchetypeChunk& chunk, usize index)
        {
            return *reinterpret_cast<Entity*>(&chunk[archetype->entityArrayOffset + index * sizeof(Entity)]);
        }

        FY_FINLINE Entity* GetEntities(Archetype* archetype, const ArchetypeChunk& chunk)
        {
            return reinterpret_cast<Entity*>(&chunk[archetype->entityArrayOffset]);
        }

        FY_FINLINE usize AddEntityChunk(Archetype* archetype, const ArchetypeChunk& chunk, Entity entity)
        {
            u32& count = GetEntityCount(archetype, chunk);
            u32  index = count++;
            GetEntity(archetype, chunk, index) = entity;
            return index;
        }

        FY_FINLINE VoidPtr GetChunkComponentData(const ArchetypeType& archetypeType, const ArchetypeChunk& chunk, usize index)
        {
            return &chunk[archetypeType.dataOffset + (index * archetypeType.typeSize)];
        }

        template<typename T>
        FY_FINLINE T* GetChunkComponentArray(const ArchetypeType& archetypeType, const ArchetypeChunk& chunk)
        {
            return reinterpret_cast<T*>(&chunk[archetypeType.dataOffset]);
        }
    }

    using ArchetypeHashMap = HashMap<usize, Array<SharedPtr<Archetype>>>;
    using ArchetypeChunk = u8*;

    struct EntityData
    {
        Entity         entity = 0;
        Archetype*     archetype = nullptr;
        ArchetypeChunk chunk = nullptr;
        u32            chunkIndex = 0;
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
                return entity;
            }

            Entity entity = entityCount++;

            new(&GetDataList(Page(entity))[Offset(entity)]) EntityData{
                .entity = entity,
                .archetype = nullptr,
                .chunk = nullptr,
                .chunkIndex = 0,
            };

            return entity;
        }

        void Destroy(Entity entity)
        {
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

        const EntityData* GetSafe(Entity entity) const
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
        constexpr static u64 ListSize = 8192;
        usize                entityCount = 1;

        Array<EntityData*> entities;
        Array<Entity>      freeEntities; //TODO: improve freeEntities list


        constexpr static u64 Page(const u64 value)
        {
            return usize{value / ListSize};
        }

        constexpr static usize Offset(const u64 value)
        {
            return usize{value & (ListSize - 1u)};
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

    struct ArchetypeQuery
    {
        Archetype* archetype;
        u8 columns[ArchetypeMaxComponents];
    };

    struct QueryCreation
    {
        usize        hash;
        Span<TypeID> types;
    };

    struct QueryData
    {
        usize                 hash;
        World*                world;
        Array<TypeID>         types;
        Array<ArchetypeQuery> archetypes;
    };

    using QueryHashMap = HashMap<usize, Array<SharedPtr<QueryData>>>;

    struct System
    {
        virtual      ~System() = default;
        virtual void OnCreate(World& world) {}
        virtual void OnUpdate(World& world) {}
        virtual void OnPostUpdate(World& world) {}
    };

    class FY_API World final
    {
    public:

        FY_NO_COPY_CONSTRUCTOR(World);

        World()
        {
            rootArchetype = CreateArchetype(0, nullptr, 0);
        }

        void AddComponents(Entity entity, TypeID* types, VoidPtr* components, i16 size)
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
                    TypeID ids[ArchetypeMaxComponents];
                    usize  i = 0;
                    for (ArchetypeType& typ : entityData->archetype->types)
                    {
                        ids[i++] = typ.typeId;
                    }
                    for (usize j = 0; j < size; j++)
                    {
                        ids[i++] = types[j];
                    }
                    MoveEntity(entity, *entityData, FindOrCreateArchetype(ids, entityData->archetype->types.Size() + size));
                }

                for (int i = 0; i < size; ++i)
                {
                    const TypeID   typeId = types[i];
                    usize          typeIndex = entityData->archetype->typeIndex.Find(typeId)->second;
                    ArchetypeType& type = entityData->archetype->types[typeIndex];

                    // ComponentState& state = FY_CHUNK_COMPONENT_STATE(type, entityContainer.chunk, entityContainer.chunkIndex);
                    // ComponentState& chunkState = FY_CHUNK_STATE(typeIndex, entityContainer.archetype, entityContainer.chunk);
                    //
                    // state.lastChange = currentStageCount;
                    // state.lastCheck = 0;
                    // chunkState.lastChange = currentStageCount;

                    VoidPtr data = Internal::GetChunkComponentData(type, entityData->chunk, entityData->chunkIndex);

                    if (type.isTriviallyCopyable)
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
                }
            }
        }

        FY_FINLINE Entity SpawnWithComponents(Entity parent, TypeID* types, VoidPtr* components, i16 size)
        {
            Entity entity = entities.CreateEntity();
            AddComponents(entity, types, components, size);
            return entity;
        }

        FY_FINLINE Entity Spawn()
        {
            Entity entity = entities.CreateEntity();
            AddComponents(entity, nullptr, nullptr, 0);
            return entity;
        }

        template <typename... Types>
        FY_FINLINE Entity Spawn(Types&&... types)
        {
            FixedArray<TypeID, sizeof...(Types)>  ids{GetTypeID<Types>()...};
            FixedArray<VoidPtr, sizeof...(Types)> components{&types...};
            return SpawnWithComponents(NullEntity, ids.begin(), components.begin(), sizeof...(Types));
        }

        template <typename... Types>
        void Add(Entity entity, Types&&... values)
        {
            FixedArray<TypeID, sizeof...(Types)>  ids{GetTypeID<Types>()...};
            FixedArray<VoidPtr, sizeof...(Types)> components{&values...};
            AddComponents(entity, ids.begin(), components.begin(), sizeof...(Types));
        }

        template <typename... Types>
        void Remove(Entity entity)
        {
            FixedArray<TypeID, sizeof...(Types)>  ids{GetTypeID<Types>()...};
            RemoveComponents(entity, ids.begin(), sizeof...(Types));
        }

        ConstPtr Get(Entity entity, TypeID typeId) const
        {
            if (const EntityData* data = entities.GetSafe(entity))
            {
                if (data->archetype != nullptr && data->chunk != nullptr)
                {
                    if (auto it = data->archetype->typeIndex.Find(typeId))
                    {
                        return Internal::GetChunkComponentData(data->archetype->types[it->second], data->chunk, data->chunkIndex);
                    }
                }
            }
            return nullptr;
        }

        template <typename Type>
        const Type* Get(Entity entity) const
        {
            return static_cast<const Type*>(Get(entity, GetTypeID<Type>()));
        }

        template <typename Type>
        bool Has(Entity entity) const
        {
            return Get(entity, GetTypeID<Type>()) != nullptr;
        }

        bool Alive(Entity entity) const
        {
            if (const EntityData* data = entities.GetSafe(entity))
            {
                return data->archetype != nullptr;
            }
            return false;
        }

        bool RemoveComponents(Entity entity, TypeID* types, usize size)
        {
            if (EntityData* data = entities.GetSafe(entity))
            {
                usize newSize = 0;
                TypeID ids[ArchetypeMaxComponents];

                for(ArchetypeType& type : data->archetype->types)
                {
                    bool found = false;
                    for (int i = 0; i < size; ++i)
                    {
                        if (types[i] == type.typeId)
                        {
                            found = true;
                            break;
                        }
                    }
                    //not found on list to remove. keep it
                    if (!found)
                    {
                        ids[newSize++] = type.typeId;
                    }
                }

                //types to remove not found
                if (newSize != data->archetype->types.Size())
                {
                    Archetype* archetype = FindOrCreateArchetype(ids, newSize);
                    MoveEntity(entity, *data, archetype);
                }
            }
            return false;
        }


        void Destroy(Entity entity)
        {
            if (EntityData* data = entities.GetSafe(entity))
            {
                RemoveEntity(entity, *data);

                data->archetype = nullptr;
                data->chunk = nullptr;
                data->chunkIndex = 0;

                entities.Destroy(entity);
            }
        }

        template<typename ...T>
        decltype(auto) Query()
        {
            using QueryImpl = Fyrion::Query<T...>;
            return QueryImpl(FindOrCreateQuery(QueryImpl::GetCreation()));
        }

        void Update();

        ~World()
        {
            for(auto& it: archetypes)
            {
                for(auto& archetype: it.second)
                {
                    for(auto& chunk : archetype->chunks)
                    {
                        for(auto& type: archetype->types)
                        {
                            if (!type.isTriviallyCopyable)
                            {
                                usize count = Internal::GetEntityCount(archetype.Get(), chunk);
                                for (usize e = 0; e < count; ++e)
                                {
                                    type.typeHandler->Destructor(Internal::GetChunkComponentData(type, chunk, e));
                                }
                            }
                        }
                        MemoryGlobals::GetDefaultAllocator().MemFree(chunk);
                    }
                }
            }
        }

    private:
        EntityStorage    entities;
        ArchetypeHashMap archetypes;
        Archetype*       rootArchetype = nullptr;
        QueryHashMap     queries;
        u64              worldFrameCount = 1;


        ArchetypeChunk FindOrCreateChunk(Archetype* archetype)
        {
            if (ArchetypeChunk activeChunk = !archetype->chunks.Empty() ? archetype->chunks.Back() : nullptr)
            {
                if (u32 count = Internal::GetEntityCount(archetype, activeChunk); count < archetype->maxEntityChunkCount)
                {
                    return activeChunk;
                }
            }

            ArchetypeChunk chunk = static_cast<ArchetypeChunk>(MemoryGlobals::GetDefaultAllocator().MemAlloc(archetype->chunkTotalAllocSize, 1));
            MemSet(chunk, 0, archetype->chunkTotalAllocSize);
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
                FY_ASSERT(false, "more then one archetype with same hash!");
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
                            .isTriviallyCopyable = typeHandler->GetTypeInfo().isTriviallyCopyable,
                        };

                        stride += typeHandler->GetTypeInfo().size;
                        continue;
                    }
                    FY_ASSERT(false, "type not found");
                }

                archetype->entityArrayOffset = 0;
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
                archetype->entityArrayOffset = 0;
                archetype->maxEntityChunkCount = ChunkComponentSize / sizeof(Entity);
                archetype->chunkTotalAllocSize = archetype->maxEntityChunkCount * sizeof(Entity);
                archetype->entityCountOffset = archetype->chunkTotalAllocSize;
                archetype->chunkTotalAllocSize += sizeof(u32);
            }

            for(auto& it: queries)
            {
                for(SharedPtr<QueryData>& data: it.second)
                {
                    CheckQueryArchetype(data.Get(), archetype);
                }
            }

            return archetype;
        }

        void RemoveEntity(Entity entity, EntityData& entityData)
        {
            ArchetypeChunk activeChunk = entityData.archetype->chunks.Back();
            u32&           entityCount = Internal::GetEntityCount(entityData.archetype, activeChunk);

            //if the entity is the last entity of the active chunk, just delete it
            if (entityCount -1  == 0 && activeChunk == entityData.chunk)
            {
                for (ArchetypeType& type : entityData.archetype->types)
                {
                    VoidPtr data = Internal::GetChunkComponentData(type, entityData.chunk, entityData.chunkIndex);
                    if (!type.isTriviallyCopyable)
                    {
                        type.typeHandler->Destructor(data);
                    }
                }

                MemoryGlobals::GetDefaultAllocator().MemFree(activeChunk);
                entityData.archetype->chunks.PopBack();
                return;
            }

            u32            lastIndex = entityCount - 1;
            Entity         lastEntity = Internal::GetEntity(entityData.archetype, activeChunk, lastIndex);

            if (lastEntity != entity)
            {
                for (ArchetypeType& type : entityData.archetype->types)
                {
                    VoidPtr src = Internal::GetChunkComponentData(type, activeChunk, lastIndex);
                    VoidPtr dst = Internal::GetChunkComponentData(type, entityData.chunk, entityData.chunkIndex);

                    if (!type.isTriviallyCopyable)
                    {
                        type.typeHandler->Move(src, dst);
                    }
                    else
                    {
                        MemCopy(dst, src, type.typeSize);
                    }

                    // FY_CHUNK_COMPONENT_STATE(type, entityData.chunk, entityData.chunkIndex) = FY_CHUNK_COMPONENT_STATE(type, activeChunk, lastIndex);
                    // type.sparse->Emplace(lastEntity, dst);
                }
            }

            Internal::GetEntity(entityData.archetype, entityData.chunk, entityData.chunkIndex) = lastEntity;

            if (EntityData* lastData = entities.GetSafe(lastEntity))
            {
                lastData->chunk = entityData.chunk;
                lastData->chunkIndex = entityData.chunkIndex;
            }

            entityCount--;

            if (entityCount == 0)
            {
                MemoryGlobals::GetDefaultAllocator().MemFree(activeChunk);
                entityData.archetype->chunks.PopBack();
            }
        }

        void MoveEntity(Entity entity, EntityData& entityData, Archetype* newArchetype)
        {
            if (entityData.archetype == newArchetype) return;
            ArchetypeChunk newChunk = FindOrCreateChunk(newArchetype);

            u32& entityCount = Internal::GetEntityCount(newArchetype, newChunk);
            u32  newIndex = entityCount++;

            //move to the new chunk, destroy not used components
            for (usize t = 0; t < entityData.archetype->types.Size(); ++t)
            {
                ArchetypeType& type = entityData.archetype->types[t];

                VoidPtr src = Internal::GetChunkComponentData(type, entityData.chunk, entityData.chunkIndex);
                if (auto it = newArchetype->typeIndex.Find(type.typeId))
                {
                    ArchetypeType& destType = newArchetype->types[it->second];
                    VoidPtr        dst = Internal::GetChunkComponentData(destType, newChunk, newIndex);

                    if (type.isTriviallyCopyable)
                    {
                        MemCopy(dst, src, destType.typeSize);
                    }
                    else
                    {
                        destType.typeHandler->Move(src, dst);
                    }
                    //FY_CHUNK_COMPONENT_STATE(destType, newChunk, newIndex) = FY_CHUNK_COMPONENT_STATE(type, entityData.chunk, entityData.chunkIndex);
                    //destType.sparse->Emplace(entity, src);
                }
            }

            RemoveEntity(entity, entityData);

            entityData.archetype = newArchetype;
            entityData.chunk = newChunk;
            entityData.chunkIndex = newIndex;
        }

        QueryData*  FindOrCreateQuery(const QueryCreation& queryCreation);

        static void CheckQueryArchetype(QueryData* queryData, Archetype* archetype);
    };
}
