#include "SceneObject.hpp"

#include "SceneManager.hpp"
#include "Assets/SceneObjectAsset.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "SceneTypes.hpp"

namespace Fyrion
{
    SceneObject::SceneObject() {}

    SceneObject::SceneObject(SceneObjectAsset* asset) : root(true), asset(asset) {}

    SceneObject::~SceneObject()
    {
        for (SceneObject* child : children)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(child);
        }
    }

    Component& SceneObject::AddComponent(TypeID typeId)
    {
        return AddComponent(Registry::FindTypeById(typeId));
    }

    Component& SceneObject::AddComponent(TypeHandler* typeHandler)
    {
        Component* component = typeHandler->Cast<Component>(typeHandler->NewInstance());
        component->typeHandler = typeHandler;
        component->object = this;
        components.EmplaceBack(component);

        if (!notificationDisabled)
        {
            TypeID typeId = typeHandler->GetTypeInfo().typeId;
            Notify(SceneNotifications_OnComponentAdded, &typeId);
            component->OnNotify(SceneNotifications_OnComponentCreated, &typeId);
        }

        return *component;
    }

    Component& SceneObject::CloneComponent(const Component* originComponent)
    {
        TypeHandler* typeHandler = originComponent->typeHandler;
        Component*   component = typeHandler->Cast<Component>(typeHandler->NewInstance());
        typeHandler->Copy(originComponent, component);

        component->typeHandler = typeHandler;
        component->object = this;
        components.EmplaceBack(component);


        if (!notificationDisabled)
        {
            TypeID typeId = typeHandler->GetTypeInfo().typeId;
            Notify(SceneNotifications_OnComponentAdded, &typeId);
            component->OnNotify(SceneNotifications_OnComponentCreated, &typeId);
        }
        return *component;
    }

    void SceneObject::RemoveComponent(Component* component)
    {
        usize index = components.IndexOf(component);
        if (index != nPos)
        {
            components.Remove(index);
        }
    }

    Span<Component*> SceneObject::GetComponents() const
    {
        return components;
    }

    Component* SceneObject::GetComponent(TypeID typeId) const
    {
        for (Component* component : components)
        {
            if (component->typeHandler->GetTypeInfo().typeId == typeId)
            {
                return component;
            }
        }

        return nullptr;
    }

    StringView SceneObject::GetName() const
    {
        if (asset)
        {
            return asset->GetName();
        }

        return name;
    }

    void SceneObject::SetName(const StringView& p_name)
    {
        if (asset)
        {
            asset->SetName(p_name);
        }
        else
        {
            name = p_name;
        }
    }

    SceneObject* SceneObject::GetParent() const
    {
        return parent;
    }

    Span<SceneObject*> SceneObject::GetChildren() const
    {
        return children;
    }

    void SceneObject::SetUUID(UUID p_uuid)
    {
        uuid = p_uuid;
    }

    UUID SceneObject::GetUUID() const
    {
        if (uuid)
        {
            return uuid;
        }

        if (asset)
        {
            return asset->GetUUID();
        }

        return {};
    }

    SceneObjectAsset* SceneObject::GetPrototype() const
    {
        if (prototype)
        {
            return prototype;
        }

        if (parent)
        {
            return parent->GetPrototype();
        }

        return nullptr;
    }

    void SceneObject::AddChild(SceneObject* sceneObject)
    {
        sceneObject->parent = this;
        children.EmplaceBack(sceneObject);
    }

    void SceneObject::AddChildAt(SceneObject* sceneObject, usize pos)
    {
        sceneObject->parent = this;
        children.Insert(children.begin() + pos, &sceneObject, &sceneObject + 1);
    }

    void SceneObject::RemoveChild(SceneObject* sceneObject)
    {
        if (const auto it = FindFirst(children.begin(), children.end(), sceneObject))
        {
            children.Erase(it);
        }
    }

    void SceneObject::RemoveChildAt(usize pos)
    {
        children.Remove(pos);
    }

    void SceneObject::Destroy()
    {
        for (SceneObject* child : children)
        {
            child->Destroy();
        }
        SceneManager::Destroy(this);
        children.Clear();
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {
        type.Constructor<SceneObjectAsset*>();

        type.Field<&SceneObject::name>("name");
    }

    ArchiveObject SceneObject::Serialize(ArchiveWriter& writer) const
    {
        ArchiveObject object = writer.CreateObject();
        writer.WriteString(object, "name", name);

        if (uuid)
        {
            writer.WriteString(object, "uuid", ToString(uuid));
        }

        if (!prototype)
        {
            ArchiveObject objChildren = writer.CreateArray();
            for (const SceneObject* child : children)
            {
                writer.AddValue(objChildren, child->Serialize(writer));
            }

            if (!children.Empty())
            {
                writer.WriteValue(object, "children", objChildren);
            }

            ArchiveObject objComponents = writer.CreateArray();

            for (const Component* component : components)
            {
                ArchiveObject componentObject = Serialization::Serialize(component->typeHandler, writer, component);
                writer.WriteString(componentObject, "_type", component->typeHandler->GetName());
                writer.AddValue(objComponents, componentObject);
            }

            if (!components.Empty())
            {
                writer.WriteValue(object, "components", objComponents);
            }
        }
        else
        {
            writer.WriteString(object, "prototype", ToString(prototype->GetUUID()));
        }

        return object;
    }

    void SceneObject::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        name = reader.ReadString(object, "name");
        notificationDisabled = true;

        if (StringView uuidStr = reader.ReadString(object, "uuid"); !uuidStr.Empty())
        {
            uuid = UUID::FromString(uuidStr);
        }

        if (ArchiveObject childrenArr = reader.ReadObject(object, "children"))
        {
            usize childrenArrSize = reader.ArrSize(childrenArr);

            ArchiveObject childObject{};
            for (usize i = 0; i < childrenArrSize; ++i)
            {
                childObject = reader.Next(childrenArr, childObject);

                SceneObject* child;
                if (StringView prototypeStr = reader.ReadString(childObject, "prototype"); !prototypeStr.Empty())
                {
                    child = SceneManager::CreateObject(AssetDatabase::FindById<SceneObjectAsset>(UUID::FromString(prototypeStr)));
                }
                else
                {
                    child = SceneManager::CreateObject();
                }

                child->Deserialize(reader, childObject);
                AddChild(child);
            }
        }

        if (ArchiveObject compArr = reader.ReadObject(object, "components"))
        {
            usize compArrSize = reader.ArrSize(compArr);

            ArchiveObject compObj{};
            for (usize i = 0; i < compArrSize; ++i)
            {
                compObj = reader.Next(compArr, compObj);

                if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(compObj, "_type")))
                {
                    Component& component = AddComponent(typeHandler);
                    Serialization::Deserialize(typeHandler, reader, compObj, &component);
                }
            }
        }
        notificationDisabled = false;
    }

    bool SceneObject::IsAlive() const
    {
        return alive;
    }

    void SceneObject::SetAlive(bool p_alive)
    {
        alive = p_alive;
    }

    void SceneObject::Notify(i64 type, VoidPtr userData)
    {
        for (Component* component : components)
        {
            component->OnNotify(type, userData);
        }

        for (SceneObject* child : children)
        {
            child->Notify(type, userData);
        }
    }

    SceneObject* SceneObject::Clone() const
    {
        SceneObject* object = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>();
        object->SetUUID(UUID::RandomUUID());
        object->SetName(GetName());
        object->prototype = object->prototype ? prototype : asset;
        object->parent = parent;

        for (const SceneObject* child : children)
        {
            object->AddChild(child->Clone());
        }

        for (const Component* component : components)
        {
            object->CloneComponent(component);
        }

        return object;
    }
}
