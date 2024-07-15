#include "MaterialAsset.hpp"

namespace Fyrion
{
    void MaterialAsset::RegisterType(NativeTypeHandler<MaterialAsset>& type)
    {
        type.Field<&MaterialAsset::baseColor>("baseColor");
        type.Field<&MaterialAsset::baseColorTexture>("baseColorTexture");
        type.Field<&MaterialAsset::normalTexture>("normalTexture");
        type.Field<&MaterialAsset::normalMultiplier>("normalMultiplier");
        type.Field<&MaterialAsset::metallic>("metallic");
        type.Field<&MaterialAsset::metallicTexture>("metallicTexture");
        type.Field<&MaterialAsset::roughness>("roughness");
        type.Field<&MaterialAsset::roughnessTexture>("roughnessTexture");
        type.Field<&MaterialAsset::metallicRoughnessTexture>("metallicRoughnessTexture");
        type.Field<&MaterialAsset::aoTexture>("aoTexture");
        type.Field<&MaterialAsset::emissiveTexture>("emissiveTexture");
        type.Field<&MaterialAsset::emissiveFactor>("emissiveFactor");
        type.Field<&MaterialAsset::alphaCutoff>("alphaCutoff");
        type.Field<&MaterialAsset::alphaMode>("alphaMode");
        type.Field<&MaterialAsset::uvScale>("uvScale");
    }
}
