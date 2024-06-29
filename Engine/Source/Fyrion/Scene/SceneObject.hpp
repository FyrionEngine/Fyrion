#pragma once

#include "Component.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class SceneObjectAsset;
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

        FY_FINLINE StringView GetName() const
        {
            return name;
        }

        FY_FINLINE void SetName(const StringView& p_name)
        {
            name = p_name;
        }

        FY_FINLINE SceneObject* GetParent() const
        {
            return parent;
        }

        FY_FINLINE Array<SceneObject*>& GetChildren()
        {
            return children;
        }

        FY_FINLINE UUID GetUUID()
        {
            return {};
        }

        SceneObjectAsset* GetPrototype()
        {
            return nullptr;
        }

    private:
        String name;
        Array<Component*> components{};
        Array<SceneObject*> children{};
        SceneObject* parent = nullptr;
    };
}
