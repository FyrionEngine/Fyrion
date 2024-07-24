#pragma once

#include "Component.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class RenderGraph;
}

namespace Fyrion
{
    class SceneObjectAsset;
    class TypeHandler;

    class FY_API SceneObject final
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(SceneObject);

        SceneObject();
        SceneObject(SceneObjectAsset* asset);
        ~SceneObject();

        StringView         GetName() const;
        void               SetName(const StringView& p_name);
        SceneObject*       GetParent() const;
        Span<SceneObject*> GetChildren() const;
        SceneObject*       FindChildByName(const StringView& p_name) const;
        SceneObject*       FindChildByUUID(const UUID& p_uuid) const;
        void               SetUUID(UUID p_uuid);
        UUID               GetUUID() const;
        SceneObject*       GetPrototype() const;
        void               AddChild(SceneObject* sceneObject);
        void               AddChildAt(SceneObject* sceneObject, usize pos);
        void               RemoveChild(SceneObject* sceneObject);
        void               RemoveChildAt(usize pos);
        Component&         AddComponent(TypeID typeId);
        Component&         AddComponent(TypeHandler* typeHandler);
        Component&         CloneComponent(const Component* component);
        void               RemoveComponent(Component* component);
        Span<Component*>   GetComponents() const;
        Component*         GetComponent(TypeID typeId) const;
        void               Destroy();
        ArchiveObject      Serialize(ArchiveWriter& writer) const;
        void               Deserialize(ArchiveReader& reader, ArchiveObject object);
        bool               IsAlive() const;
        void               SetAlive(bool p_alive);
        void               Notify(i64 type, VoidPtr userData);
        SceneObject*       Clone() const;
        void               SetPrototype(SceneObject* p_prototype);
        void               OverridePrototypeComponent(const Component* component);
        bool               IsComponentOverride(const Component* component) const;
        void               RemoveOverridePrototypeComponent(Component* component);
        bool               HasPrototypeOverride() const;

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T& AddComponent()
        {
            return static_cast<T&>(AddComponent(GetTypeID<T>()));
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T* GetComponent()
        {
            return static_cast<T*>(GetComponent(GetTypeID<T>()));
        }

        static void RegisterType(NativeTypeHandler<SceneObject>& type);

    private:
        bool                root = false;
        SceneObjectAsset*   asset{};
        SceneObject*        prototype{};
        String              name;
        UUID                uuid;
        Array<Component*>   components{};
        Array<SceneObject*> children{};
        HashSet<UUID>       componentOverride{};
        SceneObject*        parent = nullptr;
        bool                alive = true;
        bool                notificationDisabled = false;
    };
}
