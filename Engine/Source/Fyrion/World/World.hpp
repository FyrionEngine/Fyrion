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
                Archetype* archetype = FindOrCreateArchetype({types, size});
                entityContainer.chunk = FindOrCreateChunk(archetype);
                //entityContainer.chunkIndex = entityContainer.chunk->entityCount++;
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

        FY_FINLINE ArchetypeChunk* FindOrCreateChunk(Archetype* archetype)
        {
            return nullptr;
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

            Archetype* archetype = m_archetypes.Emplace(ArchetypeLookup{sorted}, MakeUnique<Archetype>()).first->second.Get();

            if (!sorted.Empty())
            {
                archetype->types.Resize(sorted.Size());

                usize stride = 0;

                for (int i = 0; i < sorted.Size(); ++i)
                {
                    TypeID typeId = sorted[i];

                    TypeHandler* typeHandler = Registry::FindTypeById(typeId);
                    FY_ASSERT(typeHandler, "Component is not registered");
                    if (typeHandler == nullptr) continue;

                    archetype->typeIndex[typeId] = i;
                    archetype->types[i].typeId = typeId;
                    archetype->types[i].typeHandler = typeHandler;
                    archetype->types[i].sparse = FindOrCreateEntitySparse(typeId);

                    stride += typeHandler->GetTypeInfo().size;
                }
                archetype->maxEntityChunkCount = Max(FY_CHUNK_SIZE / stride, (usize) 1);
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