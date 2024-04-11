#pragma once

#include "Fyrion/Common.hpp"
#include "WorldTypes.hpp"
#include "Archetype.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/FixedArray.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{

    class FY_API World final
    {
    public:
        FY_FINLINE World(const StringView& name)
        {
            m_name = name;
            m_rootArchetype = CreateArchetype({});
        }

        FY_FINLINE virtual ~World()
        {
        }

        World(const World&) = delete;
        World& operator=(const World& world) = delete;

        FY_FINLINE StringView  GetName() const { return m_name; }

        FY_FINLINE Entity Spawn()
        {
            return m_entityCounter++;
        }

        FY_FINLINE Entity Spawn(const RID& entityAsset, Entity parent, TypeID* types, VoidPtr* components, usize size)
        {
            Entity entity = m_entityCounter++;
            Add(entity, types, components, size);

            return entity;
        }

        template<typename ...Types>
        FY_FINLINE Entity Spawn(Types&&...types)
        {
            FixedArray<TypeID, sizeof...(Types)> ids{GetTypeID<Types>()...};
            FixedArray<VoidPtr,  sizeof...(Types)> components{&types...};
            return Spawn({}, NullEntity, ids.begin(), components.begin(), sizeof...(Types));
        }


        ConstPtr Get(Entity entity, TypeID typeId)
        {
            if (m_entityContainers.Size() <= entity) return nullptr;
            EntityContainer& entityContainer = m_entityContainers[entity];
            if (entityContainer.chunk && entityContainer.archetype)
            {
                if (auto it = entityContainer.archetype->typeIndex.Find(typeId))
                {
                    return FY_CHUNK_COMPONENT_DATA(entityContainer.archetype->types[it->second], entityContainer.chunk, entityContainer.chunkIndex);
                }
            }

            return nullptr;
        }

        template<typename T>
        const T& Get(Entity entity)
        {
            return *static_cast<const T*>(Get(entity, GetTypeID<T>()));
        }

        void Remove(Entity entity, TypeID* types, usize size)
        {
        }

        void Add(Entity entity, TypeID* types, VoidPtr* components, usize size)
        {
            EntityContainer& entityContainer = FindOrCreateEntityContainer(entity);

            if (entityContainer.chunk == nullptr)
            {

                entityContainer.archetype = FindOrCreateArchetype({types, size});
                entityContainer.chunk = FindOrCreateChunk(entityContainer.archetype);
                u32& entityCount = FY_CHUNK_ENTITY_COUNT(entityContainer.archetype, entityContainer.chunk);
                entityContainer.chunkIndex = entityCount++;
                FY_CHUNK_ENTITY_SET(entityContainer.archetype, entityContainer.chunk, entityContainer.chunkIndex, entity);
            }
            else
            {
                //Move Entity.
            }

            for (int i = 0; i < size; ++i)
            {
                const TypeID typeId = types[i];
                usize typeIndex = entityContainer.archetype->typeIndex.Find(typeId)->second;
                ArchetypeType& type = entityContainer.archetype->types[typeIndex];

                ComponentState& state = FY_CHUNK_COMPONENT_STATE(type, entityContainer.chunk, entityContainer.chunkIndex);
                ComponentState& chunkState = FY_CHUNK_STATE(typeIndex, entityContainer.archetype, entityContainer.chunk);

                state.lastChange = m_currentFrame;
                state.lastCheck = 0;
                chunkState.lastChange = m_currentFrame;

                CharPtr data = FY_CHUNK_COMPONENT_DATA(type, entityContainer.chunk, entityContainer.chunkIndex);
                if (!type.isTriviallyCopyable)
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
                else
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

                type.sparse->Emplace(entity, data);
            }
        }

        FY_FINLINE EntityContainer& FindOrCreateEntityContainer(Entity entity)
        {
            //TODO - improve it.
            if (m_entityContainers.Size() <= entity)
            {
                m_entityContainers.Resize(entity + 1000);
            }
            return m_entityContainers[entity];
        }

        FY_FINLINE CharPtr FindOrCreateChunk(Archetype* archetype)
        {
            if (archetype->activeChunk && archetype->maxEntityChunkCount > (u32)archetype->activeChunk[archetype->entityCountOffset])
            {
                return archetype->activeChunk;
            }
            CharPtr chunk = static_cast<CharPtr>(m_allocator.MemAlloc(archetype->chunkAllocSize, 1));
            MemSet(chunk, 0, archetype->chunkDataSize);
            archetype->activeChunk = chunk;
            return chunk;
        }

        FY_FINLINE ComponentSparse* FindOrCreateEntitySparse(TypeID typeId)
        {
            auto it = m_sparses.Find(typeId);
            if (it == m_sparses.end())
            {
                it = m_sparses.Emplace(typeId, MakeUnique<ComponentSparse>()).first;
            }
            return it->second.Get();
        }

        FY_FINLINE Archetype* FindArchetype(const Span<TypeID>& types)
        {
            if (auto it = m_archetypes.Find(types))
            {
                return it->second.Get();
            }
            return nullptr;
        }

        FY_FINLINE Archetype* FindOrCreateArchetype(const Span<TypeID>& types)
        {
            Archetype* archetypePtr = FindArchetype(types);
            if (archetypePtr == nullptr)
            {
                archetypePtr = CreateArchetype(types);
            }
            return archetypePtr;
        }

        Archetype* CreateArchetype(const Span<TypeID>& types)
        {
            FixedArray<TypeID, MaxComponentsOnChunk> arrSorted = types; //TODO remove dups

            Sort(arrSorted.begin(), arrSorted.begin() + types.Size(), [](TypeID a, TypeID b)
            {
                return a > b;
            });

            Span<TypeID> sorted = {arrSorted.begin(), arrSorted.begin() + types.Size()};

            if (Archetype* archetype = FindArchetype(sorted))
            {
                return archetype;
            }

            usize index = m_archetypes.Size();
            Archetype* archetype = m_archetypes.Emplace(ArchetypeLookup{sorted}, MakeUnique<Archetype>()).first->second.Get();
            archetype->hashId = Sum(sorted.begin(), sorted.end());
            archetype->index = index;

            if (!sorted.Empty())
            {
                archetype->types.Resize(sorted.Size());

                u32 stride = 0;

                for (int i = 0; i < sorted.Size(); ++i)
                {
                    TypeID typeId = sorted[i];

                    TypeHandler* typeHandler = Registry::FindTypeById(typeId);
                    FY_ASSERT(typeHandler, "Component is not registered");
                    if (typeHandler == nullptr) continue;
                    TypeInfo typeInfo = typeHandler->GetTypeInfo();

                    archetype->typeIndex[typeId] = i;
                    archetype->types[i].typeId = typeId;
                    archetype->types[i].typeHandler = typeHandler;
                    archetype->types[i].sparse = FindOrCreateEntitySparse(typeId);
                    archetype->types[i].isTriviallyCopyable = typeInfo.isTriviallyCopyable;
                    stride += (i32) typeInfo.size;
                }
                archetype->maxEntityChunkCount = Max(FY_CHUNK_COMPONENT_SIZE / stride, 1u);

                archetype->chunkAllocSize = (archetype->maxEntityChunkCount * sizeof(Entity));
                archetype->entityCountOffset = archetype->chunkAllocSize;
                archetype->chunkAllocSize += sizeof(u32);
                archetype->chunkStateOffset = archetype->chunkAllocSize;
                archetype->chunkAllocSize += (sorted.Size() * sizeof(ComponentState));
                archetype->chunkDataSize = archetype->chunkAllocSize;

                for (int i = 0; i < sorted.Size(); ++i)
                {
                    archetype->types[i].dataOffset = archetype->chunkAllocSize;
                    archetype->chunkAllocSize += (archetype->maxEntityChunkCount * archetype->types[i].typeHandler->GetTypeInfo().size);
                    archetype->types[i].stateOffset = archetype->chunkAllocSize;
                    archetype->chunkAllocSize += (archetype->maxEntityChunkCount * sizeof(ComponentState));
                }
            }
            else
            {
                archetype->maxEntityChunkCount = FY_CHUNK_COMPONENT_SIZE / sizeof(Entity);
                archetype->chunkAllocSize = (archetype->maxEntityChunkCount * sizeof(Entity));
                archetype->entityCountOffset = archetype->chunkAllocSize;
                archetype->chunkAllocSize += sizeof(u32);
            }

            return archetype;
        }

    private:
        Allocator& m_allocator = MemoryGlobals::GetDefaultAllocator();
        String m_name{};
        std::atomic_uint64_t m_entityCounter{1};
        HashMap<ArchetypeLookup, UniquePtr<Archetype>> m_archetypes{};
        Archetype* m_rootArchetype{};
        HashMap<TypeID, UniquePtr<ComponentSparse>> m_sparses{};
        Array<EntityContainer> m_entityContainers{};
        u64 m_currentFrame{};


        void MoveEntity(EntityContainer& entityContainer, Archetype* newArchetype)
        {
            if (entityContainer.archetype == newArchetype) return;
            CharPtr newChunk = FindOrCreateChunk(newArchetype);

            u32& entityCount = FY_CHUNK_ENTITY_COUNT(newArchetype, newChunk);
            u32 newIndex = entityCount++;

        }
    };

}