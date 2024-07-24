#include "TransformComponent.hpp"

#include <Fyrion/Scene/SceneTypes.hpp>

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void TransformComponent::OnNotify(i64 type, VoidPtr userData)
    {
        if (type == SceneNotifications_TransformChanged || type == SceneNotifications_OnActivate)
        {
            if (object->GetParent() != nullptr)
            {
                if (TransformComponent* parentTransform = object->GetParent()->GetComponent<TransformComponent>())
                {
                    worldTransform = parentTransform->GetWorldTransform() * GetLocalTransform();
                    return;
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
        type.Field<&TransformComponent::position, &TransformComponent::GetPosition, &TransformComponent::SetPosition>("position");
        type.Field<&TransformComponent::rotation, &TransformComponent::GetRotation, &TransformComponent::SetRotation>("rotation");
        type.Field<&TransformComponent::scale, &TransformComponent::GetScale, &TransformComponent::SetScale>("scale");
    }

}
