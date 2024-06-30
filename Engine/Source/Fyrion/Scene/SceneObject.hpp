#pragma once

#include "Component.hpp"
#include "Fyrion/Common.hpp"
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

        void              SetUUID(UUID p_uuid);
        UUID              GetUUID() const;
        SceneObjectAsset* GetPrototype() const;
        void              AddChild(SceneObject* sceneObject);
        void              AddChildAt(SceneObject* sceneObject, usize pos);
        void              RemoveChild(SceneObject* sceneObject);
        void              Destroy();

        static void RegisterType(NativeTypeHandler<SceneObject>& type);

    private:
        String              name;
        UUID                uuid;
        Array<Component*>   components{};
        Array<SceneObject*> children{};
        SceneObject*        parent = nullptr;
    };
}
