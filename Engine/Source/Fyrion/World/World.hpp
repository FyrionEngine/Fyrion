#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "System.hpp"

namespace Fyrion
{
    class World;
    using Entity = u64;
    constexpr u8            ArchetypeMaxComponents = 128;
    constexpr u16           ChunkComponentSize = 16 * 1024;
    constexpr static Entity NullEntity = 0;

    template<typename ...Types>
    struct Query;

    struct ComponentState
    {
        u64 tickCheck = 0;
    };

    struct ArchetypeType
    {
        usize        index;
        TypeID       typeId;
        TypeHandler* typeHandler;
        usize        typeSize;
        bool         isTriviallyCopyable;
        usize        dataOffset;
        usize        stateOffset;
    };

    struct ArchetypeChunk;

    struct Archetype
    {
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

    struct ArchetypeChunk
    {
        Archetype* archetype = nullptr;
        u8*        data = nullptr;

        explicit operator bool() const noexcept
        {
            return this->archetype != nullptr || this->data != nullptr;
        }


        FY_FINLINE u32& GetEntityCount() const
        {
            return *reinterpret_cast<u32*>(&data[archetype->entityCountOffset]);
        }

        FY_FINLINE Entity& GetEntity(usize index) const
        {
            return *reinterpret_cast<Entity*>(&data[archetype->entityArrayOffset + index * sizeof(Entity)]);
        }

        FY_FINLINE Entity* GetEntities() const
        {
            return reinterpret_cast<Entity*>(&data[archetype->entityArrayOffset]);
        }

        FY_FINLINE usize AddEntityChunk(Entity entity) const
        {
            u32& count = GetEntityCount();
            u32  index = count++;
            GetEntity(index) = entity;
            return index;
        }

        FY_FINLINE VoidPtr GetComponent(const ArchetypeType& archetypeType, usize index) const
        {
            return &data[archetypeType.dataOffset + (index * archetypeType.typeSize)];
        }

        FY_FINLINE void SetComponentState(const ArchetypeType& archetypeType, usize entityIndex, const ComponentState& componentState) const
        {
            new(&data[archetypeType.stateOffset + (entityIndex * sizeof(ComponentState))]) ComponentState{componentState};
            ComponentState& chunkState = *reinterpret_cast<ComponentState*>(&data[archetype->chunkStateOffset + archetypeType.index * sizeof(ComponentState)]);
            chunkState.tickCheck = componentState.tickCheck;
        }

        const ComponentState& GetComponentState(const ArchetypeType& archetypeType, usize entityIndex) const
        {
            return *reinterpret_cast<ComponentState*>(&data[archetypeType.stateOffset + (entityIndex * sizeof(ComponentState))]);
        }

        FY_FINLINE void AdvanceComponentState(const ArchetypeType& archetypeType, usize entityIndex)
        {
            ComponentState& componentState = *reinterpret_cast<ComponentState*>(&data[archetypeType.stateOffset + (entityIndex * sizeof(ComponentState))]);
            componentState.tickCheck++;
            ComponentState& chunkState = *reinterpret_cast<ComponentState*>(&data[archetype->chunkStateOffset + archetypeType.index * sizeof(ComponentState)]);
            chunkState.tickCheck = componentState.tickCheck;
        }

        FY_FINLINE bool IsChunkDirty(u64 tickCount, const ArchetypeType& archetypeType) const
        {
            ComponentState& chunkState = *reinterpret_cast<ComponentState*>(&data[archetype->chunkStateOffset + archetypeType.index * sizeof(ComponentState)]);
            return chunkState.tickCheck > tickCount;
        }

        FY_FINLINE bool IsEntityDirty(u64 tickCount, const ArchetypeType& archetypeType, usize entityIndex) const
        {
            ComponentState& componentState = *reinterpret_cast<ComponentState*>(&data[archetypeType.stateOffset + (entityIndex * sizeof(ComponentState))]);
            return componentState.tickCheck > tickCount;
        }

        template<typename T>
        FY_FINLINE T* GetComponentArray(const ArchetypeType& archetypeType)
        {
            return reinterpret_cast<T*>(&data[archetypeType.dataOffset]);
        }
    };

    using ArchetypeHashMap = HashMap<usize, Array<SharedPtr<Archetype>>>;

    struct EntityData
    {
        Entity          entity;
        ArchetypeChunk  chunk;
        u32             chunkIndex;
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
                .chunk = {},
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
        usize                hash = 0;
        Array<TypeID>        types;
        Array<Array<TypeID>> without;
    };

    struct QueryData
    {
        usize                 hash;
        World*                world;
        Array<TypeID>         types;
        Array<Array<TypeID>>  without;
        Array<ArchetypeQuery> archetypes;
    };

    using QueryHashMap = HashMap<usize, Array<SharedPtr<QueryData>>>;


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
                if (entityData->chunk.data == nullptr || entityData->chunk.archetype == nullptr)
                {
                    entityData->chunk = FindOrCreateChunk(FindOrCreateArchetype(types, size));
                    entityData->chunkIndex = entityData->chunk.AddEntityChunk(entity);
                }
                else
                {
                    TypeID ids[ArchetypeMaxComponents];
                    usize  i = 0;
                    for (ArchetypeType& typ : entityData->chunk.archetype->types)
                    {
                        ids[i++] = typ.typeId;
                    }
                    for (usize j = 0; j < size; j++)
                    {
                        ids[i++] = types[j];
                    }
                    MoveEntity(entity, *entityData, FindOrCreateArchetype(ids, entityData->chunk.archetype->types.Size() + size));
                }

                for (int i = 0; i < size; ++i)
                {
                    const TypeID   typeId = types[i];
                    usize          typeIndex = entityData->chunk.archetype->typeIndex.Find(typeId)->second;
                    ArchetypeType& type = entityData->chunk.archetype->types[typeIndex];

                    entityData->chunk.SetComponentState(type, entityData->chunkIndex, ComponentState{
                                                            .tickCheck = tick
                                                        });

                    VoidPtr data = entityData->chunk.GetComponent(type,entityData->chunkIndex);

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
                if (data->chunk.archetype != nullptr && data->chunk.data != nullptr)
                {
                    if (auto it = data->chunk.archetype->typeIndex.Find(typeId))
                    {
                        return data->chunk.GetComponent(data->chunk.archetype->types[it->second], data->chunkIndex);
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
                return data->chunk.archetype != nullptr;
            }
            return false;
        }

        bool RemoveComponents(Entity entity, TypeID* types, usize size)
        {
            if (EntityData* data = entities.GetSafe(entity))
            {
                usize newSize = 0;
                TypeID ids[ArchetypeMaxComponents];

                for(ArchetypeType& type : data->chunk.archetype->types)
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
                if (newSize != data->chunk.archetype->types.Size())
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

                data->chunk = {};
                data->chunkIndex = 0;

                entities.Destroy(entity);
            }
        }

        template <typename... Types>
        decltype(auto) Query()
        {
            return Fyrion::Query<Types...>(this);
        }

        FY_FINLINE u64 GetLastTick() const
        {
            return lastTick;
        }

        FY_FINLINE u64 AdvanceTick()
        {
            tick++;
            return tick;
        }

        FY_FINLINE u64 GetTick() const
        {
            return tick;
        }

        template<typename T>
        void AddSystem()
        {
            AddSystem(GetTypeID<T>());
        }

        void AddSystem(TypeID typeId);

        void Update();

        ~World();

        template <typename... T>
        friend struct Query;

    private:
        void ExecuteSystemStage(SystemExecutionStage stage);

        struct SystemStorage
        {
            TypeHandler* typeHandler{};
            SystemSetup  setup{};
            System*      system{};
            bool         initialized{};
        };


        EntityStorage        entities;
        ArchetypeHashMap     archetypes;
        Archetype*           rootArchetype = nullptr;
        QueryHashMap         queries;
        u64                  tick = 1;
        u64                  lastTick = 0;
        Array<SystemStorage> systems;
        bool                 simulating = true;


        ArchetypeChunk FindOrCreateChunk(Archetype* archetype)
        {
            if (ArchetypeChunk activeChunk = !archetype->chunks.Empty() ? archetype->chunks.Back() : ArchetypeChunk{})
            {
                if (u32 count = activeChunk.GetEntityCount(); count < archetype->maxEntityChunkCount)
                {
                    return activeChunk;
                }
            }

            ArchetypeChunk& chunk = archetype->chunks.EmplaceBack();
            chunk.archetype = archetype;
            chunk.data = static_cast<u8*>(MemoryGlobals::GetDefaultAllocator().MemAlloc(archetype->chunkTotalAllocSize, 1));
            //TODO set memory only on needed data
            MemSet(chunk.data, 0, archetype->chunkTotalAllocSize);
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
                            .index = t,
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
            ArchetypeChunk activeChunk = entityData.chunk.archetype->chunks.Back();
            u32&           entityCount = activeChunk.GetEntityCount();

            //if the entity is the last entity of the active chunk, just delete it
            if (entityCount -1  == 0 && activeChunk.data == entityData.chunk.data)
            {
                for (ArchetypeType& type : entityData.chunk.archetype->types)
                {
                    VoidPtr data = entityData.chunk.GetComponent(type, entityData.chunkIndex);
                    if (!type.isTriviallyCopyable)
                    {
                        type.typeHandler->Destructor(data);
                    }
                }

                MemoryGlobals::GetDefaultAllocator().MemFree(activeChunk.data);
                entityData.chunk.archetype->chunks.PopBack();
                return;
            }

            u32            lastIndex = entityCount - 1;
            Entity         lastEntity = activeChunk.GetEntity(lastIndex);

            if (lastEntity != entity)
            {
                for (ArchetypeType& type : entityData.chunk.archetype->types)
                {
                    VoidPtr src = activeChunk.GetComponent(type, lastIndex);
                    VoidPtr dst = entityData.chunk.GetComponent(type, entityData.chunkIndex);

                    if (!type.isTriviallyCopyable)
                    {
                        type.typeHandler->Move(src, dst);
                    }
                    else
                    {
                        MemCopy(dst, src, type.typeSize);
                    }
                    entityData.chunk.SetComponentState(type, entityData.chunkIndex, activeChunk.GetComponentState(type, lastIndex));
                }
            }

            entityData.chunk.GetEntity(entityData.chunkIndex) = lastEntity;

            if (EntityData* lastData = entities.GetSafe(lastEntity))
            {
                lastData->chunk = entityData.chunk;
                lastData->chunkIndex = entityData.chunkIndex;
            }

            entityCount--;

            if (entityCount == 0)
            {
                MemoryGlobals::GetDefaultAllocator().MemFree(activeChunk.data);
                entityData.chunk.archetype->chunks.PopBack();
            }
        }

        void MoveEntity(Entity entity, EntityData& entityData, Archetype* newArchetype)
        {
            if (entityData.chunk.archetype == newArchetype) return;
            ArchetypeChunk newChunk = FindOrCreateChunk(newArchetype);

            u32& entityCount = newChunk.GetEntityCount();
            u32  newIndex = entityCount++;

            //move to the new chunk, destroy not used components
            for (usize t = 0; t < entityData.chunk.archetype->types.Size(); ++t)
            {
                ArchetypeType& type = entityData.chunk.archetype->types[t];

                VoidPtr src = entityData.chunk.GetComponent(type, entityData.chunkIndex);
                if (auto it = newArchetype->typeIndex.Find(type.typeId))
                {
                    ArchetypeType& destType = newArchetype->types[it->second];
                    VoidPtr        dst = newChunk.GetComponent(destType, newIndex);

                    if (type.isTriviallyCopyable)
                    {
                        MemCopy(dst, src, destType.typeSize);
                    }
                    else
                    {
                        destType.typeHandler->Move(src, dst);
                    }

                    newChunk.SetComponentState(type, newIndex, entityData.chunk.GetComponentState(type, entityData.chunkIndex));
                }
            }

            RemoveEntity(entity, entityData);

            entityData.chunk = newChunk;
            entityData.chunkIndex = newIndex;
        }

        QueryData*  FindOrCreateQuery(const QueryCreation& queryCreation);
        static void CheckQueryArchetype(QueryData* queryData, Archetype* archetype);
    };
}
