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

        FY_FINLINE void Add(Entity entity, TypeID* types, VoidPtr* components, usize size)
        {
            EntityContainer& entityContainer = FindOrCreateEntityContainer(entity);

            if (entityContainer.chunk == nullptr)
            {

                entityContainer.archetype = FindOrCreateArchetype({types, size});
                entityContainer.chunk = FindOrCreateChunk(entityContainer.archetype);

                u32& entityCount = FY_CHUNK_ENTITY_COUNT(entityContainer.archetype, entityContainer.chunk);
                entityContainer.chunkIndex = entityCount++;
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

                CharPtr data = FY_CHUNK_COMPONENT_DATA(type, entityContainer.chunk, entityContainer.chunkIndex);
                ComponentState& state = FY_CHUNK_COMPONENT_STATE(type, entityContainer.chunk, entityContainer.chunkIndex);

                type.typeHandler->Copy(components[i], data);

                state.lastChange = m_currentFrame;
                state.lastCheck = 0;


               // auto memory = entityContainer.Chunk->Columns[typeIndex].Data + (entityData.ChunkIndex * typeHandler->GetTypeInfo().Size);
//
//                if (components[i])
//                {
//                    typeHandler->Copy(components[i], memory);
//                }
//                else
//                {
//                    typeHandler->Construct(memory);
//                }
//
//                ComponentState& componentState = entityData.Chunk->Columns[typeIndex].States[entityData.ChunkIndex];
//                componentState.LastChange = m_Context.CurrentFrame;
//                componentState.LastCheck = 0;
//                entityData.Chunk->ChunkStates[typeIndex].LastChange = m_Context.CurrentFrame;
//                entityData.Chunk->Archetype->Sparses[typeIndex]->Emplace(entity, memory);
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
            MemSet(chunk, 0, archetype->chunkAllocSize);
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

        FY_FINLINE Archetype* CreateArchetype(const Span<TypeID>& types)
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
    };

}