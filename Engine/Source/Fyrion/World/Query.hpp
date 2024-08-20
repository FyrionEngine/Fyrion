#pragma once

#include "Storage.hpp"
#include "Fyrion/Core/FixedArray.hpp"

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


    template <typename... Types>
    struct QueryImpl
    {
        template <typename Func>
        void Each(Func&& func)
        {

        }

        template <typename T>
        T& GetMut(Entity entity) const
        {
            return *static_cast<T*>(nullptr);
        }

        FixedArray<ComponentStorage*, sizeof...(Types)> storages;
    };
}
