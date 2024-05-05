#include "TransformComponent.hpp"

#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void TransformComponent::NotifyTransformChange() const
    {
        if (object)
        {
            object->Notify(TransformChanged);
        }
    }

    void TransformComponent::OnNotify(i64 type)
    {
        Component::OnNotify(type);
    }

    void TransformComponent::RegisterType(NativeTypeHandler<TransformComponent>& type)
    {
        type.Field<&TransformComponent::m_position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position");
        type.Field<&TransformComponent::m_rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation");
        type.Field<&TransformComponent::m_scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale");
    }
}
