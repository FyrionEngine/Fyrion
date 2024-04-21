#include "Component.hpp"

#include "SceneObject.hpp"
#include "SceneManager.hpp"

namespace Fyrion
{
    void Component::SetUpdateEnabled(bool enabled)
    {
        if (enabled)
        {
            object->GetSceneGlobals()->AddUpdatableComponent(this);
        }
        else if (m_updateIndex != U64_MAX)
        {
            object->GetSceneGlobals()->RemoveUpdatableComponent(this);
        }
    }

    void Component::RegisterType(NativeTypeHandler<Component>& type)
    {

    }
}
