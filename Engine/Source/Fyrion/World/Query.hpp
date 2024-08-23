#pragma once

#include "World.hpp"
#include <tuple>

namespace Fyrion
{
    template<typename T>
    struct QueryFunction {};


    template<typename Lambda, typename Return, typename... Params>
    struct QueryFunction<Return(Lambda::*)(Params...) const>
    {
        template<typename Func, std::size_t ...I>
        static void ForEach(QueryData* query, Func&& func, std::index_sequence<I...>)
        {
            for (ArchetypeQuery& archetypeQuery : query->archetypes)
            {
                for (ArchetypeChunk& chunk : archetypeQuery.archetype->chunks)
                {
                    usize count = Internal::GetEntityCount(archetypeQuery.archetype, chunk);
                    auto tupleComponents = std::make_tuple(Internal::GetChunkComponentArray<Traits::RemoveAll<Params>>(archetypeQuery.archetype->types[archetypeQuery.columns[I]], chunk)...);
                    //Entity* entities = Internal::GetEntities(archetypeQuery.archetype, chunk);
                    for (int e = 0; e < count; ++e)
                    {
                        func(std::get<I>(tupleComponents)[e]...);
                    }
                }
            }
        }
    };


    template <typename... Types>
    class Query
    {
    public:
        Query() = default;

        Query(QueryData* data) : data(data)
        {
        }

        template<typename Func>
        void ForEach(Func&& func)
        {
            QueryFunction<decltype(&Func::operator())>::ForEach(data, func, std::make_index_sequence<sizeof...(Types)>{});
        }

        static QueryCreation GetCreation()
        {
            static TypeID types[sizeof...(Types)] {GetTypeID<Types>()...};
            return {
                .hash = MurmurHash64(types, sizeof...(Types) * sizeof(TypeID), HashSeed64),
                .types = {types, sizeof...(Types)}
            };
        }

    private:
        QueryData* data = nullptr;
    };
}
