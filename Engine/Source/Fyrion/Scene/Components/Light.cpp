#include "Light.hpp"

#include "TransformComponent.hpp"
#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderStorage.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    void Light::OnNotify(const NotificationEvent& notificationEvent)
    {
        switch (notificationEvent.type)
        {
            case SceneNotifications_OnActivated:
            {
                if (!transformComponent)
                {
                    transformComponent = object->GetComponent<TransformComponent>();
                }

                OnChange();
                break;
            }
            case SceneNotifications_TransformChanged:
            {
                transformComponent = static_cast<TransformComponent*>(notificationEvent.component);
                OnChange();
                break;
            }
            case SceneNotifications_OnDeactivated:
            {
                switch (type)
                {
                    case LightType::Directional:
                    {
                        RenderStorage::RemoveDirectionalLight(reinterpret_cast<usize>(this));
                        break;
                    }
                    case LightType::Point:
                        break;
                    case LightType::Spot:
                        break;
                    case LightType::Area:
                        break;
                }
                break;
            }
        }
    }

    LightType Light::GetType() const
    {
        return type;
    }

    void Light::SetType(const LightType type)
    {
        this->type = type;
        OnChange();
    }

    f32 Light::GetIntensity() const
    {
        return intensity;
    }

    void Light::SetIntensity(const f32 intensity)
    {
        this->intensity = intensity;
        OnChange();
    }

    f32 Light::GetIndirectMultipler() const
    {
        return indirectMultipler;
    }

    void Light::SetIndirectMultipler(const f32 indirectMultipler)
    {
        this->indirectMultipler = indirectMultipler;
        OnChange();
    }

    bool Light::IsCastShadows() const
    {
        return castShadows;
    }

    void Light::SetCastShadows(const bool castShadows)
    {
        this->castShadows = castShadows;
        OnChange();
    }

    void Light::OnChange()
    {
        if (object->IsActivated() && transformComponent)
        {
            switch (type)
            {
                case LightType::Directional:
                {
                    RenderStorage::AddDirectionalLight(reinterpret_cast<usize>(this), DirectionalLight{
                                                           .direction = Math::MakeVec4(transformComponent->GetRotation() * Vec3::AxisY()),
                                                           .color = this->color,
                                                           .intensity = this->intensity,
                                                           .indirectMultipler = this->indirectMultipler,
                                                           .castShadows = this->castShadows,
                                                       });
                    break;
                }
                case LightType::Point:
                    break;
                case LightType::Spot:
                    break;
                case LightType::Area:
                    break;
            }
        }
    }

    void Light::RegisterType(NativeTypeHandler<Light>& type)
    {
        type.Field<&Light::type>("type").Attribute<UIProperty>();
        type.Field<&Light::color>("color").Attribute<UIProperty>();
        type.Field<&Light::intensity>("intensity").Attribute<UIProperty>();
        type.Field<&Light::indirectMultipler>("indirectMultipler").Attribute<UIProperty>();
        type.Field<&Light::castShadows>("castShadows").Attribute<UIProperty>();
    }
}
