#include "SceneObject.hpp"

namespace Fyrion
{
    SceneObject::SceneObject(SceneObject* parent, const StringView name) : m_parent(parent), m_name(name)
    {

    }

    SceneObject* SceneObject::GetScene()
    {
        if (m_parent)
        {
            return m_parent->GetScene();
        }
        return this;
    }


}
