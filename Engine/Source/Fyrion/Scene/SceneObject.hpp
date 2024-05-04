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
    class SceneGlobals;
    class SceneManager;


    class FY_API SceneObjectIterator
    {
    public:
        SceneObjectIterator(SceneObject* object);
        SceneObjectIterator begin() const;
        SceneObjectIterator end() const;

        SceneObject& operator*() const;
        SceneObject* operator->() const;

        FY_API friend bool operator==(const SceneObjectIterator& a, const SceneObjectIterator& b);
        FY_API friend bool operator!=(const SceneObjectIterator& a, const SceneObjectIterator& b);
        SceneObjectIterator& operator++();

    private:
        SceneObject* m_object;
    };


    struct ComponentStorage
    {
        TypeHandler*      typeHandler = nullptr;
        Array<Component*> instances = {};
    };

    class FY_API SceneObject
    {
    public:
        SceneObject(StringView name, SceneObject* parent);
        SceneObject(RID asset, SceneObject* parent);
        SceneObject(const SceneObject& other) = delete;
        SceneObject& operator=(const SceneObject& other) = delete;

        ~SceneObject();

        StringView          GetName() const;
        void                SetName(const StringView& newName);
        RID                 GetAsset() const;
        SceneGlobals*       GetSceneGlobals() const;
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
        void                Notify(i64 notificationId);

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

        friend class SceneManager;
        friend class SceneObjectIterator;
        friend class SceneGlobals;

    private:
        String        m_name{};
        RID           m_asset{};
        SceneObject*  m_parent{};
        SceneGlobals* m_sceneGlobals{};
        bool          m_markedToDestroy{};

        usize        m_order{};
        SceneObject* m_prev{};
        SceneObject* m_next{};

        usize        m_count{};
        SceneObject* m_head{};
        SceneObject* m_tail{};

        HashMap<TypeID, ComponentStorage> m_components{};

        SceneObject(StringView name, RID asset, SceneGlobals* sceneGlobals);

        void AddChild(SceneObject* object);
        void DestroyImmediate();
        void LoadAsset(RID asset);
    };
}
