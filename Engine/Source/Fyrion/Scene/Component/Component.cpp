
#include "Component.hpp"
#include "Fyrion/Scene/GameObject.hpp"

#include "Fyrion/Core/Registry.hpp"


namespace Fyrion
{
    void Component::RegisterType(NativeTypeHandler<Component>& type)
    {
        type.Field<&Component::gameObject>("gameObject");
        type.Field<&Component::typeId>("typeId");
    }
}
