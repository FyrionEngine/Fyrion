#include "SceneObject.hpp"

#include "SceneManager.hpp"
#include "Assets/SceneObjectAsset.hpp"
#include "GLFW/glfw3native.h"


namespace Fyrion
{
    SceneObject::SceneObject(SceneObjectAsset* asset) : asset(asset)
    {

    }

    Component& SceneObject::AddComponent(TypeID typeId)
    {
        TypeHandler* typeHandler = Registry::FindTypeById(typeId);
        Component*   component = typeHandler->Cast<Component>(typeHandler->NewInstance());
        component->typeHandler = typeHandler;
        component->object = this;
        components.EmplaceBack(component);
        return *component;
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

        ArchiveObject arr = writer.CreateArray();
        for (const SceneObject* child : GetChildren())
        {
            writer.AddValue(arr, child->Serialize(writer));
        }
        writer.WriteValue(object, "children", arr);
        return object;
    }

    void SceneObject::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        name = reader.ReadString(object, "name");

        if (StringView uuidStr = reader.ReadString(object, "uuid"); !uuidStr.Empty())
        {
            uuid = UUID::FromString(uuidStr);
        }

        ArchiveObject arr = reader.ReadObject(object, "children");
        usize arrSize = reader.ArrSize(arr);

        ArchiveObject item{};
        for (usize i = 0; i < arrSize; ++i)
        {
            item = reader.Next(arr, item);

            SceneObject* child = SceneManager::CreateObject();
            child->Deserialize(reader, item);
            AddChild(child);
        }

    }
}
