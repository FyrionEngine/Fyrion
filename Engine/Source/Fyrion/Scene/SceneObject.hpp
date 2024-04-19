#pragma once

#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    class Component;
    struct SceneManager;

    struct ComponentStorage
    {
        TypeHandler*      typeHandler;
        Array<Component*> instances;
    };

    class FY_API SceneObject
    {
    public:
        SceneObject(StringView name, SceneObject* parent, RID asset);
        SceneObject(const SceneObject& other) = delete;
        SceneObject& operator=(const SceneObject& other) = delete;

        ~SceneObject();

        SceneObject* GetScene();
        SceneObject* NewChild(const StringView& name);
        SceneObject* NewChild(const RID& asset, const StringView& name);

        Component& AddComponent(TypeID typeId);
        Component* GetComponent(TypeID typeId, u32 index);

        void RemoveChild(const SceneObject* child);

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T& AddComponent()
        {
            return static_cast<T&>(AddComponent(GetTypeID<T>()));
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T* GetComponent()
        {
            return static_cast<T*>(GetComponent(GetTypeID<T>(), 0));
        }

        void Destroy();

        static void RegisterType(NativeTypeHandler<SceneObject>& type);
        friend struct SceneManager;

    private:
        String       m_name{};
        SceneObject* m_parent{};
        usize        m_parentIndex{};
        RID          m_asset{};
        bool         m_updating{};
        bool         m_markedToDestroy{};

        Array<SceneObject*>               m_children{};
        HashMap<TypeID, ComponentStorage> m_components{};

        void DoUpdate(f64 deltaTime);
    };
}
