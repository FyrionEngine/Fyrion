#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Core/Color.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/Core/Math.hpp"


namespace Fyrion
{
    class FY_API MaterialAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<MaterialAsset>& type);

    private:
        Color     baseColor{Color::WHITE};
        Texture*  baseColorTexture{};
        Texture*  normalTexture{};
        f32       normalMultiplier{};
        f32       metallic{0.0};
        Texture*  metallicTexture{};
        f32       roughness{1.0};
        Texture*  roughnessTexture{};
        Texture*  metallicRoughnessTexture{};
        Texture*  aoTexture{};
        Texture*  emissiveTexture{};
        Vec3      emissiveFactor{};
        f32       alphaCutoff{0.0};
        AlphaMode alphaMode{};
        Vec2      uvScale{1.0f, 1.0f};
    };
}
