#pragma once

#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    class Component;
    struct SceneManager;

    struct ComponentInstace
    {
        Component* instance{};
        bool       startCalled{};
    };

    struct ComponentStorage
    {
        TypeHandler*            typeHandler = nullptr;
        Array<ComponentInstace> instances = {};
    };

    class FY_API SceneObject
    {
    public:
        SceneObject(StringView name, SceneObject* parent, RID asset);
        SceneObject(const SceneObject& other) = delete;
        SceneObject& operator=(const SceneObject& other) = delete;

        ~SceneObject();

        SceneObject*       GetScene();
        SceneObject*       GetParent() const;
        SceneObject*       NewChild(const StringView& name);
        SceneObject*       NewChild(const RID& asset, const StringView& name);
        SceneObject*       Duplicate() const;
        Span<SceneObject*> GetChildren() const;
        Component&         AddComponent(TypeID typeId);
        Component*         GetComponent(TypeID typeId, u32 index);
        u32                GetComponentTypeCount(TypeID typeId) const;
        u32                GetComponentCount() const;
        void               RemoveComponent(TypeID typeId, u32 index);
        void               RemoveChild(const SceneObject* child);
        void               Destroy();

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

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T* GetComponent(const u32 index)
        {
            return static_cast<T*>(GetComponent(GetTypeID<T>(), index));
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        void RemoveComponent()
        {
            RemoveComponent(GetTypeID<T>(), 0);
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        void RemoveComponent(const u32 index)
        {
            RemoveComponent(GetTypeID<T>(), index);
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        u32 GetComponentTypeCount() const
        {
            return GetComponentTypeCount(GetTypeID<T>());
        }

        static void RegisterType(NativeTypeHandler<SceneObject>& type);
        friend struct SceneManager;

    private:
        String       m_name{};
        SceneObject* m_parent{};
        usize        m_parentIndex{};
        RID          m_asset{};
        bool         m_updating{};
        bool         m_markedToDestroy{};
        bool         m_componentDirty{};

        Array<SceneObject*>               m_children{};
        HashMap<TypeID, ComponentStorage> m_components{};

        void DoUpdate(f64 deltaTime);
        void DoStart();
        void SetComponentDirty();
    };
}
