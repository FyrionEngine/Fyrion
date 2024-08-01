#pragma once
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion {
    class TextureAsset;
}

namespace Fyrion
{
    class PanoramicSky : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        void OnChange() override;

        void          SetSphericalTexture(TextureAsset* sphericalTexture);
        TextureAsset* GetSphericalTexture() const;

        static void RegisterType(NativeTypeHandler<PanoramicSky>& type);

    private:
        TextureAsset* sphericalTexture = nullptr;

        void GenerateCubemap();
    };
}
