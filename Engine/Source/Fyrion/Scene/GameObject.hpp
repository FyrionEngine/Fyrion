#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Scene;
    class Component;

    class FY_API GameObject
    {
    public:
        friend class Scene;

        ~GameObject();

        Scene*      GetScene() const;
        GameObject* GetParent() const;
        StringView  GetName() const;
        void        SetName(StringView newName);
        UUID        GetUUID() const;

        GameObject*       GetPrototype() const;

        GameObject*       CreateChild();
        GameObject*       CreateChildWithUUID(UUID uuid);
        Span<GameObject*> GetChildren() const;
        void              RemoveChild(GameObject* gameObject);

        Component*       GetComponent(TypeID typeId) const;
        void             GetComponentsOfType(TypeID typeId, Array<Component*> arrComponents) const;
        Component*       AddComponent(TypeID typeId);
        void             RemoveComponent(Component* component);
        Span<Component*> GetComponents() const;

        void Notify(i64 notification);
        void Destroy();

        template<typename T>
        T* GetComponent()
        {
            return static_cast<T*>(GetComponent(GetTypeID<T>()));
        }

        friend class Scene;

    private:
        GameObject(Scene* scene);
        GameObject(Scene* scene, GameObject* parent);

        Scene*      scene;
        GameObject* parent;
        String      name;
        UUID        uuid;

        Array<GameObject*> children;

        Array<Component*> components;
    };
}
