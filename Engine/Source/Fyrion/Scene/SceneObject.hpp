#pragma once

#include "Component.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashMap.hpp"

namespace Fyrion
{
    class TypeHandler;

    class FY_API SceneObject
    {
    public:
        Component& AddComponent(TypeID typeId);

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T& AddComponent()
        {
            return static_cast<T&>(AddComponent(GetTypeID<T>()));
        }

    private:
        Array<Component*> components{};
    };
}
