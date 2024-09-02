

#include "World.hpp"

namespace Fyrion
{
    QueryData* World::FindOrCreateQuery(const QueryCreation& queryCreation)
    {
        auto it = queries.Find(queryCreation.hash);
        if (it == queries.end())
        {
            it = queries.Emplace(queryCreation.hash, {}).first;
        }

        for (const SharedPtr<QueryData>& data : it->second)
        {
            if (data->types == queryCreation.types && data->without == queryCreation.without)
            {
                return data.Get();
            }
        }

        SharedPtr<QueryData>& data = it->second.EmplaceBack(MakeShared<QueryData>(queryCreation.hash, this, queryCreation.types, queryCreation.without));

        for(auto& aIt : archetypes)
        {
            for(const SharedPtr<Archetype>& archetype : aIt.second)
            {
                CheckQueryArchetype(data.Get(), archetype.Get());
            }
        }

        return data.Get();
    }

    void World::CheckQueryArchetype(QueryData* queryData, Archetype* archetype)
    {
        ArchetypeQuery archetypes{
            .archetype = archetype
        };

        if (!queryData->without.Empty())
        {
            u32 countFound = 0;

            for (u32 i = 0; i < queryData->without.Size(); ++i)
            {
                bool found = false;

                for (u32 j = 0; j < queryData->without[i].Size(); ++j)
                {
                    TypeID type = queryData->without[i][j];
                    if (archetype->typeIndex.Has(type))
                    {
                        found = true;
                        break;
                    }
                }
                if (found)
                {
                    countFound++;
                }
            }

            if (countFound == queryData->without.Size())
            {
                return;
            }
        }

        for (u32 i = 0; i < queryData->types.Size(); ++i)
        {
            TypeID type = queryData->types[i];

            if (auto it = archetype->typeIndex.Find(type))
            {
                archetypes.columns[i] = it->second;
            }
            else
            {
                return;
            }
        }
        queryData->archetypes.EmplaceBack(archetypes);
    }

    void World::AddSystem(TypeID typeId)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            System* system = typeHandler->Cast<System>(typeHandler->NewInstance());
            FY_ASSERT(system, "system cannot be created");
            if (system)
            {
                system->world = this;
                systems.EmplaceBack(SystemStorage{
                    .typeHandler = typeHandler,
                    .system = system,
                    .initialized = false
                });
            }
        }
    }

    void World::ExecuteSystemStage(SystemExecutionStage stage)
    {
        //update stage
        for(SystemStorage& systemStorage : systems)
        {
            if (systemStorage.setup.stage == stage &&
                ((simulating && systemStorage.setup.policy == SystemExecutionPolicy::Simulation) || systemStorage.setup.policy == SystemExecutionPolicy::Update) &&
                systemStorage.system)
            {
                systemStorage.system->OnUpdate();
            }
        }
    }

    void World::Update()
    {
        //init stage
        for(SystemStorage& systemStorage : systems)
        {
            if (systemStorage.system)
            {
                if (!systemStorage.initialized)
                {
                    systemStorage.system->OnInit(systemStorage.setup);
                    systemStorage.initialized = true;
                }
            }
        }

        ExecuteSystemStage(SystemExecutionStage::OnPreUpdate);
        ExecuteSystemStage(SystemExecutionStage::OnUpdate);
        ExecuteSystemStage(SystemExecutionStage::OnPostUpdate);

        lastTick = tick;
    }

    World::~World()
    {
        //destroy stage
        for(SystemStorage& systemStorage : systems)
        {
            if (systemStorage.system)
            {
                systemStorage.system->OnDestroy();
                systemStorage.typeHandler->Destroy(systemStorage.system);
            }
        }

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
                            usize count = chunk.GetEntityCount();
                            for (usize e = 0; e < count; ++e)
                            {
                                type.typeHandler->Destructor(chunk.GetComponent(type, e));
                            }
                        }
                    }
                    MemoryGlobals::GetDefaultAllocator().MemFree(chunk.data);
                }
            }
        }
    }


    void WorldTypeRegister()
    {
        Registry::Type<System>();
        Registry::Type<World>();
    }

}
