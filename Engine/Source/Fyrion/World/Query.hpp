#pragma once

#include "World.hpp"
#include <tuple>

namespace Fyrion
{
    template<typename...Ts>
    using QueryTupleCat = decltype(std::tuple_cat(std::declval<Ts>()...));


    template<typename T>
    struct RefMut
    {
        using type = T;
    };

    template<typename T>
    struct Changed
    {
        using type = T;
    };

    template<typename T>
    struct Without
    {
    };

    template<typename T>
    struct QueryFilter
    {
        using type = T;
    };



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
                    usize count = chunk.GetEntityCount();
                    auto tupleComponents = std::make_tuple(chunk.GetChunkComponentArray<Traits::RemoveAll<Params>>(archetypeQuery.archetype->types[archetypeQuery.columns[I]])...);
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
      // using TupleTypes = QueryTupleCat<std::conditional_t<QueryTypeFilter<Types>::value, std::tuple<typename Types::type>, std::tuple<>>...>;

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
