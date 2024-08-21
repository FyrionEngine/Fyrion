#pragma once

#include "Storage.hpp"
#include "Fyrion/Core/FixedArray.hpp"

#include <tuple>

namespace Fyrion
{

    template<typename Type>
    struct FilterType
    {
        using type = Type;
    };


    template<typename ...T>
    struct Changed
    {
    };

    template<typename Type>
    struct Without
    {
        using type = Type;
    };


    template <typename Type>
    struct StorageValidator
    {
        static bool Valid(ComponentStorage* storage, Entity entity)
        {
            return storage->Has(entity);
        }
    };

    template <typename Type>
    struct StorageValidator<Without<Type>>
    {
        static bool Valid(ComponentStorage* storage, Entity entity)
        {
            return !storage->Has(entity);
        }
    };

    template <typename T>
    struct QueryEach {};

    template <typename... Types>
    struct QueryImpl
    {
        using TupleTypes = decltype(std::make_tuple(std::declval<Types>()...));
        FixedArray<ComponentStorage*, sizeof...(Types)> storages;

        template<typename Func>
        void Each(Func&& func)
        {
            QueryEach<decltype(&Func::operator())>::Each(Traits::Forward<Func>(func), *this);
        }

        template <typename T>
        T& GetMut(Entity entity) const
        {
            return *static_cast<T*>(nullptr);
        }
    };


    template<typename C, typename Type, typename ...Types>
    struct QueryEach<void(C::*)(Entity, Type, Types...) const>
    {




        template<typename Func, typename Query>
        static void Each(Func&& func, Query& query)
        {
            ComponentStorage* storage = query.storages[Traits::TupleIndex<Traits::RemoveAll<Type>, typename Query::TupleTypes>::value];

            for (EntityChunk chunk : storage->GetChunks())
            {
                for (int e = 0; e < Internal::ChunkGetCount(chunk); ++e)
                {
                    Entity entity = Internal::ChunkGetEntities(chunk)[e];
                    if ((query.storages[Traits::TupleIndex<Traits::RemoveAll<Types>, typename Query::TupleTypes>::value]->Has(entity) && ...))
                    {
                        func(entity,
                            *static_cast<const Traits::RemoveAll<Type>*>(storage->Get(entity)),
                            *static_cast<const Traits::RemoveAll<Types>*>(query.storages[Traits::TupleIndex<Traits::RemoveAll<Types>, typename Query::TupleTypes>::value]->Get(entity)) ...);
                    }
                }
            }




            // for(const auto& entity :  m_pools[0]->Packed())
            // {
            // 	if (((0 == Index || m_pools[Index]->Has(entity)) && ... ))
            // 	{
            // 		func(entity, m_pools[Index]->template Get<Types>(entity)...);
            // 	}
            // }
        }
    };
}
