#pragma once
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/Scene/Component.hpp"

namespace Fyrion
{
    class TransformComponent;

    class FY_API Light : public Component
    {
    public:
        FY_BASE_TYPES(Component);


        LightType GetType() const;
        void      SetType(LightType type);
        f32       GetIntensity() const;
        void      SetIntensity(f32 intensity);
        f32       GetIndirectMultipler() const;
        void      SetIndirectMultipler(f32 indirectMultipler);
        bool      IsCastShadows() const;
        void      SetCastShadows(bool castShadows);


        void OnNotify(const NotificationEvent& notificationEvent) override;
        void OnChange() override;


        static void RegisterType(NativeTypeHandler<Light>& type);

    private:
        LightType type = LightType::Directional;
        Color     color = Color::WHITE;
        f32       intensity = 2.0;
        f32       indirectMultipler = 1.0;
        bool      castShadows = false;

        TransformComponent* transformComponent = nullptr;
    };
}
