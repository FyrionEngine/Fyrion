#pragma once

#include <tuple>

#include "World.hpp"

namespace Fyrion
{

    namespace Internal
    {

        template<typename TupleType>
        constexpr static void GetTupleTypeIds(TypeID* ids, std::index_sequence<>)
        {
        }

        template<typename TupleType, usize I, usize ...Is>
        constexpr static void GetTupleTypeIds(TypeID* ids, std::index_sequence<I, Is...>)
        {
            ids[I] = GetTypeID<std::tuple_element_t<I, TupleType>>();
            GetTupleTypeIds<TupleType>(ids, std::index_sequence<Is...>{});
        }
    }


    template <typename T>
    struct ReadWrite
    {
        using type = T;
        T& value;

        ReadWrite(T& value) : value(value) {}

        bool updated = false;

        ReadWrite& operator=(const T& value)
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
        using Tuple = decltype(std::make_tuple(std::declval<T>()));
        using ValidationTuple = decltype(std::tuple());

        template <typename TupleType>
        FY_FINLINE static decltype(auto) GetChunkData(ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return chunk.GetComponentArray<T>(archetypeQuery.archetype->types[archetypeQuery.columns[Traits::TupleIndex<T, TupleType>::value]]);
        }
    };

    template <>
    struct ComponentHandler<Entity>
    {
        template <typename TupleType>
        FY_FINLINE static decltype(auto) GetChunkData(ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return chunk.GetEntities();
        }
    };

    template <typename ...T>
    struct ComponentHandler<Changed<T...>>
    {
        using Tuple = decltype(std::make_tuple(std::declval<T>()...));
        using ValidationTuple = decltype(std::make_tuple(std::declval<Changed<T...>>()));

        template<typename TupleType>
        FY_FINLINE static bool IsValidChunk(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return (chunk.IsChunkDirty(query->world->GetTickCount(), archetypeQuery.archetype->types[archetypeQuery.columns[Traits::TupleIndex<T, TupleType>::value]]) && ...);
        }

        template<typename TupleType>
        FY_FINLINE static bool IsValidEntity(QueryData* query, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk, u32 entityIndex)
        {
            return (chunk.IsEntityDirty(query->world->GetTickCount(), archetypeQuery.archetype->types[archetypeQuery.columns[Traits::TupleIndex<T, TupleType>::value]], entityIndex) && ...);
        }
    };


    struct QueryData;

    template<typename T>
    struct QueryFunction {};

    template<typename Lambda, typename Return, typename... Params>
    struct QueryFunction<Return(Lambda::*)(Params...) const>
    {
        template<typename... QueryTypes, typename Func, std::size_t ...V>
        static void ForEach(Query<QueryTypes...>& query, Func&& func, std::index_sequence<V...>)
        {
            using ValidationTypes = typename Query<QueryTypes...>::ValidationTypes;
            using TupleTypes = typename Query<QueryTypes...>::TupleTypes;

            for (ArchetypeQuery& archetypeQuery : query.data->archetypes)
            {
                for (ArchetypeChunk& chunk : archetypeQuery.archetype->chunks)
                {
                    if ((ComponentHandler<std::tuple_element_t<V, ValidationTypes>>::template IsValidChunk<TupleTypes>(query.data, archetypeQuery, chunk) || ...))
                    {
                        auto tupleComponents = std::make_tuple(ComponentHandler<Traits::RemoveAll<Params>>::template GetChunkData<TupleTypes>(archetypeQuery, chunk)...);
                        usize count = chunk.GetEntityCount();
                        for (int e = 0; e < count; ++e)
                        {
                            if ((ComponentHandler<std::tuple_element_t<V, ValidationTypes>>::template IsValidEntity<TupleTypes>(query.data, archetypeQuery, chunk, e) || ...))
                            {
                                func(static_cast<const Traits::RemoveAll<Params>&>(std::get<Traits::RemoveAll<Params>*>(tupleComponents)[e])...);
                            }
                        }
                    }
                }
            }
        }

        template<typename... QueryTypes, typename Func>
        static void ForEach(Query<QueryTypes...>& query, Func&& func, std::index_sequence<>)
        {
            using TupleTypes = typename Query<QueryTypes...>::TupleTypes;

            for (ArchetypeQuery& archetypeQuery : query.data->archetypes)
            {
                for (ArchetypeChunk& chunk : archetypeQuery.archetype->chunks)
                {
                    auto tupleComponents = std::make_tuple(ComponentHandler<Traits::RemoveAll<Params>>::template GetChunkData<TupleTypes>(archetypeQuery, chunk)...);

                    usize count = chunk.GetEntityCount();
                    for (int e = 0; e < count; ++e)
                    {
                        func(static_cast<const Traits::RemoveAll<Params>&>(std::get<Traits::RemoveAll<Params>*>(tupleComponents)[e])...);
                    }
                }
            }
        }

        template <typename... QueryTypes, typename Func>
        FY_FINLINE static void ForEach(Query<QueryTypes...>& query, Func&& func)
        {
            ForEach(query, Traits::Forward<Func>(func), std::make_index_sequence<std::tuple_size_v<typename Query<QueryTypes...>::ValidationTypes>>{});
        }
    };

    template <typename... Types>
    struct Query
    {
        using TupleTypes = decltype(std::tuple_cat(std::declval<typename ComponentHandler<Types>::Tuple>()...));
        using ValidationTypes = decltype(std::tuple_cat(std::declval<typename ComponentHandler<Types>::ValidationTuple>()...));

        Query() = default;

        explicit Query(World* world)
        {
            FY_ASSERT(world, "World cannot be null");
            TypeID types[std::tuple_size_v<TupleTypes>];
            Internal::GetTupleTypeIds<TupleTypes>(types, std::make_index_sequence<std::tuple_size_v<TupleTypes>>{});

            data = world->FindOrCreateQuery({
                .hash = MurmurHash64(types, std::tuple_size_v<TupleTypes> * sizeof(TypeID), HashSeed64),
                .types = {types, std::tuple_size_v<TupleTypes>}
            });
        }

        template <typename Func>
        void ForEach(Func&& func)
        {
            FY_ASSERT(data, "query not initialized");
            QueryFunction<decltype(&Func::operator())>::ForEach(*this, func);
        }

        QueryData* data = nullptr;
    };


}
