

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
            if (data->types == queryCreation.types)
            {
                return data.Get();
            }
        }

        SharedPtr<QueryData>& data = it->second.EmplaceBack(MakeShared<QueryData>(queryCreation.hash, this, queryCreation.types));

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

    void World::Update()
    {

    }

}
