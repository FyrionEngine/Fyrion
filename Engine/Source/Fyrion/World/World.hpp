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

#include <algorithm>

namespace Fyrion
{

    class FY_API World final
    {
    public:
        FY_FINLINE World(const StringView& name)
        {
            m_name = name;
            m_rootArchetype = CreateArchetype(nullptr, 0);
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
                Archetype* archetype = FindOrCreateArchetype(Sum(types, types + size), types, size);
                entityContainer.chunk = FindOrCreateChunk(archetype);
                entityContainer.chunkIndex = entityContainer.chunk->entityCount++;
                entityContainer.chunk->entities[entityContainer.chunkIndex] = entity;
            }
            else
            {
                Insert(m_Context.TypeIdTempBuffer.begin(),
                    entityContainer.chunk->archetype->typesIds.begin(),
                    entityContainer.chunk->archetype->typesIds.end());

                Insert(m_Context.TypeIdTempBuffer.begin() +
                    entityContainer.chunk->archetype->typesIds.Size(), types, types + size);

                usize newSize = entityContainer.chunk->archetype->typesIds.Size() + size;

                Archetype* newArchetype = FindOrCreateArchetype(
                    Sum(m_Context.TypeIdTempBuffer.begin(), m_Context.TypeIdTempBuffer.end()),
                    m_Context.TypeIdTempBuffer.begin(), newSize);

                ArchetypeChunk* newArchetypeChunk = FindOrCreateChunk(newArchetype);
                MoveEntityChunk(entity, entityContainer.chunk, newArchetypeChunk);
            }

            for (int i = 0; i < size; ++i)
            {
                const TypeID typeId = types[i];
                usize typeIndex = entityContainer.chunk->archetype->typeIndex.Find(typeId)->second;
                TypeHandler* typeHandler = entityContainer.chunk->archetype->types[typeIndex];
                auto memory = entityContainer.chunk->columns[typeIndex].data + (entityContainer.chunkIndex * typeHandler->GetTypeInfo().size);

                if (components[i])
                {
                    typeHandler->Copy(components[i], memory);
                }
                else
                {
                    typeHandler->Construct(memory);
                }

                ComponentState& componentState = entityContainer.chunk->columns[typeIndex].state[entityContainer.chunkIndex];
                componentState.lastChange = m_currentFrame;
                componentState.lastCheck = 0;
                entityContainer.chunk->chunkStates[typeIndex].lastChange = m_currentFrame;
                entityContainer.chunk->archetype->sparses[typeIndex]->Emplace(entity, memory);
            }
        }

        FY_FINLINE EntityContainer& FindOrCreateEntityContainer(Entity entity)
        {
            if (m_entityContainers.Size() <= entity)
            {
                m_entityContainers.Resize(entity + 1000);
            }
            return m_entityContainers[entity];
        }

    private:
        Allocator& m_allocator = MemoryGlobals::GetDefaultAllocator();
        String m_name{};
        std::atomic_uint64_t m_entityCounter{1};
        HashMap<TypeID, Array<UniquePtr<Archetype>>> m_archetypes{};
        Archetype* m_rootArchetype{};
        HashMap<usize, UniquePtr<ComponentSparse>> m_sparses{};
        Array<EntityContainer> m_entityContainers{};
        u64 m_currentFrame{};

        FY_FINLINE ArchetypeChunk* FindOrCreateChunk(Archetype* archetype)
        {
            if (archetype->activeChunk && archetype->maxEntityChunkCount > archetype->activeChunk->entityCount)
            {
                return archetype->activeChunk;
            }

            for (auto& chunk: archetype->chunks)
            {
                if (archetype->maxEntityChunkCount > archetype->activeChunk->entityCount)
                {
                    archetype->activeChunk = chunk.Get();
                    return chunk.Get();
                }
            }

            ArchetypeChunk* archetypeChunk = archetype->chunks.EmplaceBack(MakeUnique<ArchetypeChunk>()).Get();
            archetypeChunk->archetype = archetype;

//            archetypeChunk->entities.Resize(archetype->MaxEntityChunkCount);
//            archetypeChunk->chunkStates.Resize(archetype->MaxEntityChunkCount);
//            archetypeChunk->columns.Resize(archetype->Types.Size() + 1); //last column is a null column used for OPT queries

            if (!archetype->types.Empty())
            {
//                for (int i = 0; i < archetype->types.Size(); ++i)
//                {
//                    ArchetypeChunkColumn& chunkColumn = archetypeChunk->Columns[i];
//                    chunkColumn.data = (char*) GetDefaultAllocator()->MemAlloc(GetDefaultAllocator(), archetype->ColumnAllocationSize[i]);
//                    chunkColumn.state.Resize(archetype->MaxEntityChunkCount);
//                }
            }
            archetype->activeChunk = archetypeChunk;
            return archetypeChunk;
        }

        FY_FINLINE ComponentSparse* FindOrCreateEntitySparse(TypeID typeId)
        {
            auto it = m_sparses.Find(typeId);
            if (it == m_sparses.end())
            {
                it = m_sparses.Emplace((usize) typeId, MakeUnique<ComponentSparse>()).first;
            }
            return it->second.Get();
        }

        FY_FINLINE Archetype* FindArchetype(usize hash, TypeID* types, usize size)
        {
            if (auto it = m_archetypes.Find(hash))
            {
                if (it->second.Size() > 1)
                {
                    FY_ASSERT(false, "");
                    //TODO hash collisions
                    //(find what archetype matches with ids and return according)
                    //asset added just in case it happens
                    return it->second[0].Get();
                }
                else if (!it->second.Empty())
                {
                    return it->second[0].Get();
                }
            }
            return nullptr;
        }

        FY_FINLINE Archetype* FindOrCreateArchetype(usize hash, TypeID* types, usize size)
        {
            Archetype* archetypePtr = FindArchetype(hash, types, size);
            if (archetypePtr == nullptr)
            {
                archetypePtr = CreateArchetype(types, size);
            }
            return archetypePtr;
        }

        FY_FINLINE Archetype* CreateArchetype(TypeID* types, usize size)
        {
            Array<TypeID> sortedTypes{};
            sortedTypes.Resize(size);
            for (int i = 0; i < size; ++i)
            {
                sortedTypes[i] = types[i];
            }

            Sort(sortedTypes.begin(), sortedTypes.end(), [](TypeID a, TypeID b)
            {
                return a > b;
            });

            sortedTypes.Erase(std::unique(sortedTypes.begin(), sortedTypes.end()), sortedTypes.end()); //
            usize newHash = Sum(sortedTypes.begin(), sortedTypes.end());

            auto it = m_archetypes.Find(newHash);
            if (it == m_archetypes.end())
            {
                it = m_archetypes.Emplace(newHash, Array<UniquePtr<Archetype>>{}).first;
            }
            else
            {
                //try to search with new hash sorted and without duplicates
                Archetype* archetype = FindArchetype(newHash, types, size);
                if (archetype)
                {
                    return archetype;
                }
            }
            Archetype* archetype = it->second.EmplaceBack(MakeUnique<Archetype>(newHash)).Get();

            if (!sortedTypes.Empty())
            {
                for (const TypeID typeId: sortedTypes)
                {
                    if (!archetype->typeIndex.Find(typeId))
                    {
                        TypeHandler* typeHandler = Registry::FindTypeById(typeId);
                        FY_ASSERT(typeHandler, "Component must be registered");
                        archetype->typesIds.EmplaceBack(typeId);
                        archetype->sparses.EmplaceBack(FindOrCreateEntitySparse(typeId));
                        archetype->typeIndex.Emplace(typeId, archetype->types.Size());
                        archetype->types.EmplaceBack(typeHandler);
                        archetype->typesSize.EmplaceBack(typeHandler->GetTypeInfo().size);
                        archetype->columnAllocationSize.EmplaceBack();
                        archetype->stride += typeHandler->GetTypeInfo().size;
                    }
                }
                archetype->maxEntityChunkCount = Max(FY_CHUNK_SIZE / archetype->stride, (usize) 1);
                for (int i = 0; i < archetype->typesIds.Size(); ++i)
                {
                    archetype->columnAllocationSize[i] = archetype->types[i]->GetTypeInfo().size * archetype->maxEntityChunkCount;
                }
            }
            else
            {
                archetype->maxEntityChunkCount = FY_CHUNK_SIZE / sizeof(Entity);
            }

//            for (const auto& itQuery: ueries)
//            {
//                CheckArchetype(itQuery.Second.Get(), archetype);
//            }
            return archetype;
        }
    };

}