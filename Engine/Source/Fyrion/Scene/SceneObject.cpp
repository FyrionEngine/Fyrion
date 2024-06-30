#include "SceneObject.hpp"

#include "SceneManager.hpp"


namespace Fyrion
{
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
        return uuid;
    }

    SceneObjectAsset* SceneObject::GetPrototype() const
    {
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

    void SceneObject::Destroy()
    {
        SceneManager::Destroy(this);
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {
        type.Field<&SceneObject::name>("name");
    }
}
