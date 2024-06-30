#include "SceneObject.hpp"


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

    UUID SceneObject::GetUUID() const
    {
        return {};
    }

    SceneObjectAsset* SceneObject::GetPrototype() const
    {
        return nullptr;
    }

    SceneObject* SceneObject::NewChild()
    {
        SceneObject* child = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>();
        child->parent = this;
        children.EmplaceBack(child);
        return child;
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {
        type.Field<&SceneObject::name>("name");
    }
}
