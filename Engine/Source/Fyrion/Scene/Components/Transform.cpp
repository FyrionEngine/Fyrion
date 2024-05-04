#include "Transform.hpp"

#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void Transform::NotifyTransformChange() const
    {
        if (object)
        {
            object->Notify(TransformChanged);
        }
    }

    void Transform::OnNotify(i64 type)
    {
        Component::OnNotify(type);
    }

    void Transform::RegisterType(NativeTypeHandler<Transform>& type)
    {
        type.Field<&Transform::m_position, &Transform::GetPosition, &Transform::SetPosition>("position");
        type.Field<&Transform::m_rotation, &Transform::GetRotation, &Transform::SetRotation>("rotation");
        type.Field<&Transform::m_scale, &Transform::GetScale, &Transform::SetScale>("scale");
    }
}
