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
}
