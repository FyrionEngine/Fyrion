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
        if (parent == nullptr)
        {
            return scene->GetUUID();
        }
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

    GameObject* GameObject::FindChildByName(StringView name) const
    {
        for (GameObject* child : children)
        {
            if (child->name == name)
            {
                return child;
            }
        }

        return nullptr;
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
            Component* component = static_cast<Component*>(typeHandler->NewObject());
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

    Span<Component*> GameObject::GetComponents() const
    {
        return components;
    }

    ArchiveValue GameObject::Serialize(ArchiveWriter& writer) const
    {
        if (!GetUUID())
        {
            return {};
        }

        ArchiveValue object = writer.CreateObject();

        if (!name.Empty())
        {
            writer.AddToObject(object, "name", writer.StringValue(name));
        }

        if (uuid)
        {
            writer.AddToObject(object, "uuid", writer.StringValue(uuid.ToString()));
        }

        ArchiveValue childrenArr{};

        for (GameObject* child : children)
        {
            if (ArchiveValue valueChild = child->Serialize(writer))
            {
                if (!childrenArr)
                {
                    childrenArr = writer.CreateArray();
                }
                writer.AddToArray(childrenArr, valueChild);
            }
        }

        if (childrenArr)
        {
            writer.AddToObject(object, "children", childrenArr);
        }

        ArchiveValue componentArr{};

        for (const Component* component : components)
        {
            TypeHandler* typeHandler = Registry::FindTypeById(component->typeId);

            ArchiveValue componentValue = Serialization::Serialize(typeHandler, writer, component);
            writer.AddToObject(componentValue, "_type", writer.StringValue(typeHandler->GetName()));
            if (!componentArr)
            {
                componentArr = writer.CreateArray();
            }
            writer.AddToArray(componentArr, componentValue);
        }

        if (componentArr)
        {
            writer.AddToObject(object, "components", componentArr);
        }

        return object;
    }

    void GameObject::Deserialize(ArchiveReader& reader, ArchiveValue value)
    {
        if (StringView name = reader.StringValue(reader.GetObjectValue(value, "name")); !name.Empty())
        {
            SetName(name);
        }

        ArchiveValue arrChildren = reader.GetObjectValue(value, "children");
        usize arrChildrenSize = reader.ArraySize(arrChildren);

        ArchiveValue vlChildren{};
        for (usize c = 0; c < arrChildrenSize; ++c)
        {
            vlChildren = reader.ArrayNext(arrChildren, vlChildren);
            UUID uuid = UUID::FromString(reader.StringValue(reader.GetObjectValue(vlChildren, "uuid")));

            GameObject* child = CreateChildWithUUID(uuid);
            child->Deserialize(reader, vlChildren);
        }

        ArchiveValue arrComponent = reader.GetObjectValue(value, "components");
        usize arrComponentSize = reader.ArraySize(arrComponent);


        ArchiveValue vlComponent{};
        for (usize c = 0; c < arrComponentSize; ++c)
        {
            vlComponent = reader.ArrayNext(arrComponent, vlComponent);
            if (StringView typeName = reader.StringValue(reader.GetObjectValue(vlComponent, "_type")); !typeName.Empty())
            {
                TypeHandler* typeHandler = Registry::FindTypeByName(typeName);
                if (typeHandler)
                {
                    Component* component = AddComponent(typeHandler->GetTypeInfo().typeId);
                    Serialization::Deserialize(typeHandler, reader, vlComponent, component);
                }
            }
        }
    }

    void GameObject::Notify(i64 notification)
    {
        for (Component* component : components)
        {
            component->OnNotify(notification);
        }
    }

    void GameObject::Destroy()
    {
        scene->DestroyGameObject(this);
    }
}
