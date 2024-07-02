#include "SceneObject.hpp"

#include "SceneManager.hpp"
#include "Assets/SceneObjectAsset.hpp"


namespace Fyrion
{
    SceneObject::SceneObject(SceneObjectAsset* asset) : asset(asset) {}

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

    StringView SceneObject::GetName() const
    {
        return name;
    }

    void SceneObject::SetName(const StringView& p_name)
    {
        name = p_name;
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
        return asset;
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
        SceneManager::Destroy(this);
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {
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
            ArchiveObject componentObject = component->typeHandler->Serialize(writer, component);
            writer.WriteString(componentObject, "_type", component->typeHandler->GetName());
            writer.AddValue(objComponents, componentObject);
        }

        if (!components.Empty())
        {
            writer.WriteValue(object, "components", objComponents);
        }

        return object;
    }

    void SceneObject::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        name = reader.ReadString(object, "name");

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

                SceneObject* child = SceneManager::CreateObject();
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
                    typeHandler->Deserialize(reader, compObj, &component);
                }
            }
        }
    }

    bool SceneObject::IsAlive() const
    {
        return alive;
    }

    void SceneObject::SetAlive(bool p_alive)
    {
        alive = p_alive;
    }
}
