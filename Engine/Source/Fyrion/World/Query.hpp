#pragma once

#include <tuple>

#include "World.hpp"

namespace Fyrion
{
    template <typename T>
    struct RWValue
    {
        using type = T;
        T& value;

        bool updated = false;

        RWValue& operator=(const T& value)
        {
            if (!(value == this->value))
            {
                this->value = value;
                updated = true;
            }
            return *this;
        }
    };


    template <typename ...T>
    struct Changed
    {
    };

    template <typename ...T>
    struct Without
    {
    };

    template <typename T>
    struct ComponentHandler
    {
        FY_FINLINE static bool IsValidChunk(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return true;
        }

        FY_FINLINE static bool IsValidEntity(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk, u32 entityIndex)
        {
            return true;
        }

        FY_FINLINE static void GetComponentIDs(Array<TypeID>& ids)
        {
            ids.EmplaceBack(GetTypeID<T>());
        }
    };

    template <typename ...T>
    struct ComponentHandler<Changed<T...>>
    {
        FY_FINLINE static bool IsValidChunk(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            //TODO find the index here.
            return chunk.IsChunkDirty(query->world->GetStageCount(), archetypeQuery.archetype->types[archetypeQuery.columns[0]]);
        }

        FY_FINLINE static bool IsValidEntity(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk, u32 entityIndex)
        {
            return true;
        }

        FY_FINLINE static void GetComponentIDs(Array<TypeID>& ids)
        {
            (ids.EmplaceBack(GetTypeID<T>()),...);
        }
    };

    template <typename ...T>
    struct QueryValidator
    {
        FY_FINLINE static bool IsValidChunk(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return (ComponentHandler<T>::IsValidChunk(query, archetypeQuery, chunk) && ...);
        }

        FY_FINLINE static bool IsValidEntity(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk, u32 entityIndex)
        {
            return (ComponentHandler<T>::IsValidEntity(query, archetypeQuery, chunk, entityIndex) && ...);
        }
    };

    struct QueryData;

    template<typename T>
    struct QueryFunction {};


    template<typename Lambda, typename Return, typename... Params>
    struct QueryFunction<Return(Lambda::*)(Params...) const>
    {
        template<typename... QueryTypes, typename Func>
        static void ForEach(QueryData* query, Func&& func)
        {
            ForEach<QueryTypes...>(query, func, std::make_index_sequence<sizeof...(Params)>{});
        }

        template<typename... QueryTypes, typename Func, std::size_t ...I>
        static void ForEach(Query<QueryTypes...>& query, Func&& func, std::index_sequence<I...>)
        {
            using QueryValidatorImpl = QueryValidator<QueryTypes...>;

            for (ArchetypeQuery& archetypeQuery : query.data->archetypes)
            {
                for (ArchetypeChunk& chunk : archetypeQuery.archetype->chunks)
                {
                    if (QueryValidatorImpl::IsValidChunk(query.data, archetypeQuery,  chunk))
                    {
                        auto tupleComponents = std::make_tuple(chunk.GetComponentArray<Traits::RemoveAll<Params>>(archetypeQuery.archetype->types[archetypeQuery.columns[I]])...);
                        //Entity* entities = Internal::GetEntities(archetypeQuery.archetype, chunk);

                        usize count = chunk.GetEntityCount();
                        for (int e = 0; e < count; ++e)
                        {
                            if (QueryValidatorImpl::IsValidEntity(query.data, archetypeQuery, chunk, e))
                            {
                                func(std::get<I>(tupleComponents)[e]...);
                            }
                        }
                    }
                }
            }
        }
    };

    template <typename... Types>
    struct Query
    {
        Query() = default;

        explicit Query(World* world)
        {
            FY_ASSERT(world, "World cannot be null");
            //static TypeID types[sizeof...(Types)] {GetTypeID<Types>()...};
            Array<TypeID> types;
            (ComponentHandler<Types>::GetComponentIDs(types),...);
            data = world->FindOrCreateQuery({
                .hash = MurmurHash64(types.begin(), types.Size() * sizeof(TypeID), HashSeed64),
                .types = types
            });
        }

        template <typename Func>
        void ForEach(Func&& func)
        {
            FY_ASSERT(data, "query not initialized");
            QueryFunction<decltype(&Func::operator())>::ForEach(*this, func, std::make_index_sequence<sizeof...(Types)>{});
        }

        QueryData* data = nullptr;
    };


}
