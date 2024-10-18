#include "TransformComponent.hpp"

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void TransformComponent::RegisterType(NativeTypeHandler<TransformComponent>& type)
    {
        type.Field<&TransformComponent::position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position").Attribute<UIProperty>();
        type.Field<&TransformComponent::rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation").Attribute<UIProperty>();
        type.Field<&TransformComponent::scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale").Attribute<UIProperty>();
    }
}
