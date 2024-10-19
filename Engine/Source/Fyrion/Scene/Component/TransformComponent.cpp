#include "TransformComponent.hpp"

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/GameObject.hpp"
#include "Fyrion/Scene/SceneTypes.hpp"

namespace Fyrion
{
    void TransformComponent::UpdateTransform()
    {
        if (gameObject->GetParent() != nullptr)
        {
            if (TransformComponent* parentTransform = gameObject->GetParent()->GetComponent<TransformComponent>())
            {
                UpdateTransform(parentTransform);
                return;
            }
        }
        UpdateTransform(nullptr);
    }

    void TransformComponent::UpdateTransform(const TransformComponent* parentTransform)
    {
        worldTransform = parentTransform != nullptr ? parentTransform->GetWorldTransform() * GetLocalTransform() : GetLocalTransform();

        gameObject->Notify(NotificationType::TransformChanged);

        for (GameObject* child : gameObject->GetChildren())
        {
            if (TransformComponent* childTransform = child->GetComponent<TransformComponent>())
            {
                childTransform->UpdateTransform(this);
            }
        }
    }

    void TransformComponent::OnChange()
    {
        UpdateTransform();
    }

    void TransformComponent::RegisterType(NativeTypeHandler<TransformComponent>& type)
    {
        type.Field<&TransformComponent::position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position").Attribute<UIProperty>();
        type.Field<&TransformComponent::rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation").Attribute<UIProperty>();
        type.Field<&TransformComponent::scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale").Attribute<UIProperty>();
    }


}
