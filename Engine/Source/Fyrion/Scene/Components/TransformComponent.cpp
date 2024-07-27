#include "TransformComponent.hpp"

#include <Fyrion/Scene/SceneTypes.hpp>

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void TransformComponent::OnNotify(i64 type, VoidPtr userData)
    {
        if (type == SceneNotifications_TransformChanged || type == SceneNotifications_OnActivated)
        {
            if (object->GetParent() != nullptr)
            {
                if (TransformComponent* parentTransform = object->GetParent()->GetComponent<TransformComponent>())
                {
                    worldTransform = parentTransform->GetWorldTransform() * GetLocalTransform();
                }
            }
            worldTransform = GetLocalTransform();
        }
    }

    void TransformComponent::OnChange()
    {
        object->Notify(SceneNotifications_TransformChanged, this);
    }

    void TransformComponent::RegisterType(NativeTypeHandler<TransformComponent>& type)
    {
        type.Field<&TransformComponent::position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position").Attribute<UIProperty>();
        type.Field<&TransformComponent::rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation").Attribute<UIProperty>();
        type.Field<&TransformComponent::scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale").Attribute<UIProperty>();
    }

}
