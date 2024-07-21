#include "TransformComponent.hpp"

#include <Fyrion/Scene/SceneTypes.hpp>

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void TransformComponent::OnChange()
    {
        object->Notify(SceneNotifications_TransformChanged, this);
    }

    void TransformComponent::RegisterType(NativeTypeHandler<TransformComponent>& type)
    {
        type.Field<&TransformComponent::position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position");
        type.Field<&TransformComponent::rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation");
        type.Field<&TransformComponent::scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale");
    }

}
