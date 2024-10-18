#include "GameObject.hpp"
#include "Scene.hpp"
#include "Component/Component.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    GameObject::GameObject(Scene* scene) : scene(scene), parent(nullptr) {}
    GameObject::GameObject(Scene* scene, GameObject* parent) : scene(scene), parent(parent) {}


    GameObject::~GameObject()
    {
        if (parent)
        {
            parent->RemoveChild(this);
        }

        if (scene && uuid)
        {
            scene->objectsById.Erase(uuid);
        }

        for (GameObject* child : children)
        {
            child->parent = nullptr;
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(child);
        }
    }

    Scene* GameObject::GetScene() const
    {
        return scene;
    }

    GameObject* GameObject::GetParent() const
    {
        return parent;
    }

    StringView GameObject::GetName() const
    {
        return name;
    }

    void GameObject::SetName(StringView newName)
    {
        name = newName;
    }

    UUID GameObject::GetUUID() const
    {
        return uuid;
    }

    GameObject* GameObject::GetPrototype() const
    {
        return nullptr;
    }

    Span<GameObject*> GameObject::GetChildren() const
    {
        return children;
    }

    GameObject* GameObject::CreateChild()
    {
        return CreateChildWithUUID(UUID::RandomUUID());
    }

    GameObject* GameObject::CreateChildWithUUID(UUID uuid)
    {
        GameObject* gameObject = new(MemoryGlobals::GetDefaultAllocator().MemAlloc(sizeof(GameObject), alignof(GameObject))) GameObject{scene, this};
        gameObject->uuid = uuid;
        children.EmplaceBack(gameObject);
        scene->objectsById.Insert(uuid, gameObject);
        return gameObject;
    }

    void GameObject::RemoveChild(GameObject* gameObject)
    {
        if (auto it = FindFirst(children.begin(), children.end(), gameObject))
        {
            children.Erase(it);
        }
    }

    Component* GameObject::GetComponent(TypeID typeId) const
    {
        for (Component* component : components)
        {
            if (component->typeId == typeId)
            {
                return component;
            }
        }
        return nullptr;
    }

    void GameObject::GetComponentsOfType(TypeID typeId, Array<Component*> arrComponents) const
    {
        arrComponents.Clear();
        for (Component* component : components)
        {
            if (component->typeId == typeId)
            {
                arrComponents.EmplaceBack(component);
            }
        }
    }

    Component* GameObject::AddComponent(TypeID typeId)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            Component* component = typeHandler->Cast<Component>(typeHandler->NewInstance());
            component->gameObject = this;
            component->typeId = typeId;
            components.EmplaceBack(component);

            return component;
        }
        return nullptr;
    }

    void GameObject::RemoveComponent(Component* component)
    {
        if (auto it = FindFirst(components.begin(), components.end(), component))
        {
            if (TypeHandler* typeHandler = Registry::FindTypeById(component->typeId))
            {
                typeHandler->Destroy(component);
            }
            components.Erase(it);
        }
    }

    void GameObject::Destroy()
    {
        scene->DestroyGameObject(this);
    }
}
