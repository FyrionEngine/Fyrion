#include "Component.hpp"

#include "SceneObject.hpp"
#include "SceneManager.hpp"

namespace Fyrion
{
    void Component::EnableComponentUpdate()
    {
        if (m_updateIndex == U64_MAX)
        {
            object->GetSceneGlobals()->AddUpdatableComponent(this);
        }
    }

    void Component::DisableComponentUpdate()
    {
        if (m_updateIndex != U64_MAX)
        {
            object->GetSceneGlobals()->RemoveUpdatableComponent(this);
        }
    }

    void Component::RegisterType(NativeTypeHandler<Component>& type)
    {

    }
}
