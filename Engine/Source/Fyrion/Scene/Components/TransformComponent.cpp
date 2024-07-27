#include "TransformComponent.hpp"

#include <Fyrion/Scene/SceneTypes.hpp>

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void TransformComponent::OnNotify(const NotificationEvent& notificationEvent)
    {
        switch (notificationEvent.type)
        {
            case SceneNotifications_OnActivated:
            {
                this->OnChange();
                break;
            }
            case SceneNotifications_ParentTransformChanged:
            {
                worldTransform = static_cast<TransformComponent*>(notificationEvent.component)->GetWorldTransform() * GetLocalTransform();

                object->NotifyComponents(NotificationEvent{.type = SceneNotifications_TransformChanged, .component = this});
                object->NotifyChildren(NotificationEvent{.type = SceneNotifications_ParentTransformChanged, .component = this});
                break;
            }
            default:
                break;
        }
    }

    void TransformComponent::OnChange()
    {
        if (TransformComponent* parentTransform = object->GetParent() != nullptr ? object->GetParent()->GetComponent<TransformComponent>() : nullptr)
        {
            worldTransform = parentTransform->GetWorldTransform() * GetLocalTransform();
        }
        else
        {
            worldTransform = GetLocalTransform();
        }

        object->NotifyComponents(NotificationEvent{.type = SceneNotifications_TransformChanged, .component = this});
        object->NotifyChildren(NotificationEvent{.type = SceneNotifications_ParentTransformChanged, .component = this});
    }

    void TransformComponent::RegisterType(NativeTypeHandler<TransformComponent>& type)
    {
        type.Field<&TransformComponent::position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position").Attribute<UIProperty>();
        type.Field<&TransformComponent::rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation").Attribute<UIProperty>();
        type.Field<&TransformComponent::scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale").Attribute<UIProperty>();
    }
}
