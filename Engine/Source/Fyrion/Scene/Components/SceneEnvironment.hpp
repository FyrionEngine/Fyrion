#pragma once
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion
{
    class SceneEnvironment : public Component
    {
    public:
        FY_BASE_TYPES(Component);


        void OnNotify(const NotificationEvent& notificationEvent) override;
        void OnChange() override;

        static void RegisterType(NativeTypeHandler<SceneEnvironment>& type);
    private:
        TextureAsset* skyboxTexture = nullptr;
    };
}
