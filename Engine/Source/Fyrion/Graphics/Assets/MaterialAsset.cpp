#include "MaterialAsset.hpp"

#include "Fyrion/Graphics/Graphics.hpp"
#include "ShaderAsset.hpp"

namespace Fyrion
{
    BindingSet* MaterialAsset::GetBindingSet()
    {
        if (!bindingSet)
        {
            bindingSet = Graphics::CreateBindingSet(AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/BasicRenderer.raster"));
            bindingSet->GetVar("texture")->SetTexture(baseColorTexture->GetTexture());
        }
        return bindingSet;
    }

    MaterialAsset::~MaterialAsset()
    {
        if (bindingSet)
        {
            Graphics::DestroyBindingSet(bindingSet);
        }
    }

    Color MaterialAsset::GetBaseColor() const
    {
        return baseColor;
    }

    void MaterialAsset::SetBaseColor(const Color& baseColor)
    {
        this->baseColor = baseColor;
    }

    TextureAsset* MaterialAsset::GetBaseColorTexture() const
    {
        return baseColorTexture;
    }

    void MaterialAsset::SetBaseColorTexture(TextureAsset* baseColorTexture)
    {
        this->baseColorTexture = baseColorTexture;
    }

    TextureAsset* MaterialAsset::GetNormalTexture() const
    {
        return normalTexture;
    }

    void MaterialAsset::SetNormalTexture(TextureAsset* normalTexture)
    {
        this->normalTexture = normalTexture;
    }

    f32 MaterialAsset::GetNormalMultiplier() const
    {
        return normalMultiplier;
    }

    void MaterialAsset::SetNormalMultiplier(f32 normalMultiplier)
    {
        this->normalMultiplier = normalMultiplier;
    }

    f32 MaterialAsset::GetMetallic() const
    {
        return metallic;
    }

    void MaterialAsset::SetMetallic(f32 metallic)
    {
        this->metallic = metallic;
    }

    TextureAsset* MaterialAsset::GetMetallicTexture() const
    {
        return metallicTexture;
    }

    void MaterialAsset::SetMetallicTexture(TextureAsset* metallicTexture)
    {
        this->metallicTexture = metallicTexture;
    }

    f32 MaterialAsset::GetRoughness() const
    {
        return roughness;
    }

    void MaterialAsset::SetRoughness(f32 roughness)
    {
        this->roughness = roughness;
    }

    TextureAsset* MaterialAsset::GetRoughnessTexture() const
    {
        return roughnessTexture;
    }

    void MaterialAsset::SetRoughnessTexture(TextureAsset* roughnessTexture)
    {
        this->roughnessTexture = roughnessTexture;
    }

    TextureAsset* MaterialAsset::GetMetallicRoughnessTexture() const
    {
        return metallicRoughnessTexture;
    }

    void MaterialAsset::SetMetallicRoughnessTexture(TextureAsset* metallicRoughnessTexture)
    {
        this->metallicRoughnessTexture = metallicRoughnessTexture;
    }

    TextureAsset* MaterialAsset::GetAoTexture() const
    {
        return aoTexture;
    }

    void MaterialAsset::SetAoTexture(TextureAsset* aoTexture)
    {
        this->aoTexture = aoTexture;
    }

    TextureAsset* MaterialAsset::GetEmissiveTexture() const
    {
        return emissiveTexture;
    }

    void MaterialAsset::SetEmissiveTexture(TextureAsset* emissiveTexture)
    {
        this->emissiveTexture = emissiveTexture;
    }

    Vec3 MaterialAsset::GetEmissiveFactor() const
    {
        return emissiveFactor;
    }

    void MaterialAsset::SetEmissiveFactor(const Vec3& emissiveFactor)
    {
        this->emissiveFactor = emissiveFactor;
    }

    f32 MaterialAsset::GetAlphaCutoff() const
    {
        return alphaCutoff;
    }

    void MaterialAsset::SetAlphaCutoff(f32 alphaCutoff)
    {
        this->alphaCutoff = alphaCutoff;
    }

    AlphaMode MaterialAsset::GetAlphaMode() const
    {
        return alphaMode;
    }

    void MaterialAsset::SetAlphaMode(AlphaMode alphaMode)
    {
        this->alphaMode = alphaMode;
    }

    Vec2 MaterialAsset::GetUvScale() const
    {
        return uvScale;
    }

    void MaterialAsset::SetUvScale(const Vec2& uvScale)
    {
        this->uvScale = uvScale;
    }

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
