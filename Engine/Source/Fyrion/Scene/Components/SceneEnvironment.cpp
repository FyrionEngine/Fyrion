#include "SceneEnvironment.hpp"

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Graphics/RenderStorage.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void SceneEnvironment::OnNotify(const NotificationEvent& notificationEvent)
    {
        switch (notificationEvent.type)
        {
            case SceneNotifications_OnActivated:
            {
                OnChange();
                break;
            }
            case SceneNotifications_OnDeactivated:
            {
                RenderStorage::AddSkybox(nullptr);
                break;
            }
        }
    }

    void SceneEnvironment::OnChange()
    {
        if (object->IsActivated())
        {
            RenderStorage::AddSkybox(skyboxTexture);
        }
    }

    void SceneEnvironment::RegisterType(NativeTypeHandler<SceneEnvironment>& type)
    {
        type.Field<&SceneEnvironment::skyboxTexture>("skyboxTexture").Attribute<UIProperty>();
    }
}
