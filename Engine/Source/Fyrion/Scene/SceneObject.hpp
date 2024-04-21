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
    class SceneObject;
    struct SceneManager;

    class FY_API SceneObjectIterator
    {
    public:
        SceneObjectIterator(SceneObject* object);
        SceneObjectIterator(SceneObject* object, SceneObject* next);
        SceneObjectIterator begin() const;
        SceneObjectIterator end() const;

        SceneObject& operator*() const;
        SceneObject* operator->() const;

        FY_API friend bool operator==(const SceneObjectIterator& a, const SceneObjectIterator& b);
        FY_API friend bool operator!=(const SceneObjectIterator& a, const SceneObjectIterator& b);
        SceneObjectIterator& operator++();

    private:
        SceneObject* m_object;
        SceneObject* m_next;
    };


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
        SceneObject(StringView name, SceneObject* parent);
        SceneObject(StringView name, RID asset, SceneObject* parent);
        SceneObject(const SceneObject& other) = delete;
        SceneObject& operator=(const SceneObject& other) = delete;

        ~SceneObject();

        StringView          GetName() const;
        void                SetName(const StringView& newName);
        RID                 GetAsset() const;
        SceneObject*        GetScene() const;
        SceneObject*        GetParent() const;
        SceneObject*        NewChild(const StringView& name);
        SceneObject*        NewChild(const RID& asset);
        SceneObject*        Duplicate() const;
        SceneObjectIterator GetChildren();
        u64                 GetChildrenCount() const;
        Component&          AddComponent(TypeID typeId);
        Component*          GetComponent(TypeID typeId, u32 index);
        u32                 GetComponentTypeCount(TypeID typeId) const;
        u32                 GetComponentCount() const;
        void                RemoveComponent(TypeID typeId, u32 index);
        void                RemoveChild(const SceneObject* child);
        void                Destroy();

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
        friend class SceneObjectIterator;

    private:
        String       m_name{};
        RID          m_asset{};
        SceneObject* m_parent{};
        bool         m_markedToDestroy{};
        bool         m_componentDirty{};

        SceneObject* m_prev{};
        SceneObject* m_next{};

        usize        m_count{};
        SceneObject* m_head{};
        SceneObject* m_tail{};

        HashMap<TypeID, ComponentStorage> m_components{};

        void DoUpdate(f64 deltaTime);
        void DoStart();
        void SetComponentDirty();
        void AddChild(SceneObject* object);
        void DestroyImmediate();
    };
}
