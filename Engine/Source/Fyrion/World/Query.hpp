#pragma once

#include <tuple>

#include "World.hpp"

//query need a redesign.

namespace Fyrion
{
    template <typename T>
    struct ReadWrite
    {
        using type = T;
        T& value;

        ReadWrite(T& value) : value(value) {}

        bool updated = true;

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

    template<>
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
        using PostCheckTuple = decltype(std::tuple());

        template<typename TupleType>
        FY_FINLINE static bool IsValidChunk(u64 tick, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return (chunk.IsChunkDirty(tick, archetypeQuery.archetype->types[archetypeQuery.columns[Traits::TupleIndex<T, TupleType>::value]]) && ...);
        }

        template<typename TupleType>
        FY_FINLINE static bool IsValidEntity(u64 tick, ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk, u32 entityIndex)
        {
            return (chunk.IsEntityDirty(tick, archetypeQuery.archetype->types[archetypeQuery.columns[Traits::TupleIndex<T, TupleType>::value]], entityIndex) && ...);
        }
    };

    template <typename ...T>
    struct ComponentHandler<Without<T...>>
    {
        using Tuple = decltype(std::tuple());
        using ValidationTuple = decltype(std::tuple());
    };

    template<typename T>
    struct ComponentHandler<ReadWrite<T>>
    {
        using Tuple = decltype(std::make_tuple(std::declval<T>()));
        using ValidationTuple = decltype(std::tuple());

        template <typename TupleType>
        FY_FINLINE static decltype(auto) GetChunkData(ArchetypeQuery& archetypeQuery, ArchetypeChunk& chunk)
        {
            return chunk.GetComponentArray<T>(archetypeQuery.archetype->types[archetypeQuery.columns[Traits::TupleIndex<T, TupleType>::value]]);
        }
    };

    template<typename T>
    struct TypeExtractorImpl
    {
        using Type = Traits::RemoveAll<T>;
    };

    template<typename T>
    struct TypeExtractorImpl<ReadWrite<T>>
    {
        using Type = Traits::RemoveAll<T>;
    };

    template<typename T>
    using TypeExtractor = typename TypeExtractorImpl<T>::Type;


    struct QueryData;

    template<typename T>
    struct QueryFunction {};

    struct PostExecutionCheck
    {
        template <typename... QueryTypes, typename T>
        constexpr static void Check(Query<QueryTypes...>& query, ArchetypeChunk& chunk, usize entityIndex, const T&)
        {
        }

        template<typename... QueryTypes, typename T>
        static void Check(Query<QueryTypes...>& query, ArchetypeChunk& chunk, usize entityIndex, ReadWrite<T> readWrite);
    };

    template<typename T>
    struct IsValidParam
    {
        constexpr static bool valid = true;
    };

    template<typename T>
    struct IsValidParam<const T&>
    {
        constexpr static bool valid = true;
    };

    template<typename T>
    struct IsValidParam<T&>
    {
        constexpr static bool valid = false;
    };


    template<typename Lambda, typename Return, typename... Params>
    struct QueryFunction<Return(Lambda::*)(Params...) const>
    {
        static_assert((IsValidParam<Params>::valid && ...), "param cannot be a non const reference");

        template<typename... QueryTypes, std::size_t ...V>
        constexpr static void PostCheck(Query<QueryTypes...>& query, auto& tuple, ArchetypeChunk& chunk, usize entityIndex, std::index_sequence<V...>)
        {
            (PostExecutionCheck::Check(query, chunk, entityIndex, std::get<V>(tuple)), ...);
        }

        template<typename... QueryTypes, typename Func, std::size_t ...V>
        constexpr static void ForEach(Query<QueryTypes...>& query, Func&& func, std::index_sequence<V...>)
        {
            using ValidationTypes = typename Query<QueryTypes...>::ValidationTypes;
            using TupleTypes = typename Query<QueryTypes...>::TupleTypes;

            for (ArchetypeQuery& archetypeQuery : query.data->archetypes)
            {
                for (ArchetypeChunk& chunk : archetypeQuery.archetype->chunks)
                {
                    if ((ComponentHandler<std::tuple_element_t<V, ValidationTypes>>::template IsValidChunk<TupleTypes>(query.tick, archetypeQuery, chunk) || ...))
                    {
                        auto tupleComponents = std::make_tuple(ComponentHandler<Traits::RemoveAll<Params>>::template GetChunkData<TupleTypes>(archetypeQuery, chunk)...);
                        usize count = chunk.GetEntityCount();
                        for (int e = 0; e < count; ++e)
                        {
                            if ((ComponentHandler<std::tuple_element_t<V, ValidationTypes>>::template IsValidEntity<TupleTypes>(query.tick, archetypeQuery, chunk, e) || ...))
                            {
                                auto tupleItems = std::make_tuple(static_cast<const Traits::RemoveAll<Params>&>(std::get<TypeExtractor<Params>*>(tupleComponents)[e])...);
                                std::apply(func, tupleItems);
                                PostCheck(query, tupleItems, chunk, e, std::make_index_sequence<std::tuple_size_v<decltype(tupleItems)>>{});
                            }
                        }
                    }
                }
            }
        }

        template<typename... QueryTypes, typename Func>
        constexpr static void ForEach(Query<QueryTypes...>& query, Func&& func, std::index_sequence<>)
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
                        auto tupleItems = std::make_tuple(static_cast<const Traits::RemoveAll<Params>&>(std::get<TypeExtractor<Params>*>(tupleComponents)[e])...);
                        std::apply(func, tupleItems);
                        PostCheck(query, tupleItems, chunk, e, std::make_index_sequence<std::tuple_size_v<decltype(tupleItems)>>{});
                    }
                }
            }
        }

        template <typename... QueryTypes, typename Func>
        constexpr static void ForEach(Query<QueryTypes...>& query, Func&& func)
        {
            ForEach(query, Traits::Forward<Func>(func), std::make_index_sequence<std::tuple_size_v<typename Query<QueryTypes...>::ValidationTypes>>{});
        }
    };

    template<typename >
    struct QueryTypesArr
    {
        constexpr static usize tupleSize = 0;
    };

    template<typename ...T>
    struct QueryTypesArr<std::tuple<T...>>
    {
        constexpr static usize tupleSize = sizeof...(T);
        constexpr static TypeID types[tupleSize] = {GetTypeID<T>()...};
    };

    template<>
    struct QueryTypesArr<std::tuple<>>
    {
        constexpr static usize tupleSize = 0;
    };


    template<typename>
    struct WithoutHandler
    {
        static void UpdateCreation(QueryCreation& creation)
        {
        }
    };

    template<typename ...T>
    struct WithoutHandler<Without<T...>>
    {
        static void UpdateCreation(QueryCreation& creation)
        {
            Array<TypeID> types;
            types.Reserve(sizeof...(T));
            (types.EmplaceBack(GetTypeID<T>()), ...);
            creation.without.EmplaceBack(types);
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

            QueryCreation queryCreation{};

            if constexpr (sizeof...(Types) > 0)
            {
                TypeID types[sizeof...(Types)];
                queryCreation.hash = MurmurHash64(types, sizeof...(Types) * sizeof(TypeID), HashSeed64);
            }

            if constexpr (QueryTypesArr<TupleTypes>::tupleSize > 0)
            {
                queryCreation.types = Span<TypeID>{const_cast<TypeID*>(QueryTypesArr<TupleTypes>::types), QueryTypesArr<TupleTypes>::tupleSize};
            }

            (WithoutHandler<Types>::UpdateCreation(queryCreation),...);

            data = world->FindOrCreateQuery(queryCreation);
            tick = world->GetLastTick();
        }

        template <typename Func>
        void ForEach(Func&& func)
        {
            FY_ASSERT(data, "query not initialized");
            QueryFunction<decltype(&Func::operator())>::ForEach(*this, Traits::Forward<Func>(func));
            tick = data->world->GetTick();
        }

        QueryData* data = nullptr;
        u64        tick = 0;
    };

    template <typename... QueryTypes, typename T>
    void PostExecutionCheck::Check(Query<QueryTypes...>& query, ArchetypeChunk& chunk, usize entityIndex, ReadWrite<T> readWrite)
    {
        if (readWrite.updated)
        {
            query.data->world->AdvanceTick();
            using TupleTypes = typename Query<QueryTypes...>::TupleTypes;
            chunk.AdvanceComponentState(chunk.archetype->types[Traits::TupleIndex<T, TupleTypes>::value], entityIndex);
        }
    }
}
