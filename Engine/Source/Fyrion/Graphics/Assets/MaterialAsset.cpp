#include "MaterialAsset.hpp"

#include "Fyrion/Graphics/Graphics.hpp"
#include "ShaderAsset.hpp"
#include "Fyrion/Core/Attributes.hpp"

namespace Fyrion
{
    struct MaterialData
    {
        Vec4 baseColorAlphaCutOff;
        Vec4 uvScaleNormalMultiplierAlphaMode;
        Vec4 metallicRoughness;
        Vec4 emissiveFactor;
        Vec4 textureProps; //hasNormal. hasMetallic, hasRoughness,  hasMetallicRoughness
    };

    BindingSet* MaterialAsset::GetBindingSet()
    {
        if (!bindingSet)
        {
            MaterialData materialData{
                .baseColorAlphaCutOff = Math::MakeVec4(GetBaseColor().ToVec3(), alphaCutoff),
                .uvScaleNormalMultiplierAlphaMode = Math::MakeVec4(GetUvScale(), Math::MakeVec2(GetNormalMultiplier(), static_cast<f32>(GetAlphaMode()))),
                .metallicRoughness = Vec4{GetRoughness(), GetMetallic(), 0.0f, 0.0f},
                .emissiveFactor = Math::MakeVec4(GetEmissiveFactor(), 0.0),
                .textureProps = {0.0, 0.0, 0.0, 0.0},
            };

            bindingSet = Graphics::CreateBindingSet(AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/BasicRenderer.raster"));
            bindingSet->GetVar("baseColorTexture")->SetTexture(baseColorTexture ? baseColorTexture->GetTexture() : Graphics::GetDefaultTexture());
            //bindingSet->GetVar("defaultSampler")->SetSampler(baseColorTexture ? baseColorTexture->GetSampler() : Graphics::GetDefaultSampler());


            if (normalTexture)
            {
                bindingSet->GetVar("normalTexture")->SetTexture(normalTexture->GetTexture());
                materialData.textureProps[0] = 1.0;
            }

            if (metallicTexture)
            {
                bindingSet->GetVar("metallicTexture")->SetTexture(metallicTexture->GetTexture());
                materialData.textureProps[1] = 1.0;
            }

            if (roughnessTexture)
            {
                bindingSet->GetVar("roughnessTexture")->SetTexture(roughnessTexture->GetTexture());
                materialData.textureProps[2] = 1.0;
            }

            if (metallicRoughnessTexture)
            {
                bindingSet->GetVar("metallicRoughnessTexture")->SetTexture(metallicRoughnessTexture->GetTexture());
                materialData.textureProps[3] = 1.0;
            }

            bindingSet->GetVar("material")->SetValue(&materialData, sizeof(MaterialData));
        }
        return bindingSet;
    }

    MaterialAsset::~MaterialAsset()
    {
        if (bindingSet)
        {
            Graphics::DestroyBindingSet(bindingSet);
            bindingSet = nullptr;
        }
    }

    StringView MaterialAsset::GetDisplayName() const
    {
        return "Material";
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

    void MaterialAsset::OnModified()
    {
        if (bindingSet)
        {
            Graphics::WaitQueue();
            Graphics::DestroyBindingSet(bindingSet);
            bindingSet = nullptr;
        }
    }

    void MaterialAsset::RegisterType(NativeTypeHandler<MaterialAsset>& type)
    {
        type.Field<&MaterialAsset::baseColor>("baseColor").Attribute<UIProperty>();
        type.Field<&MaterialAsset::baseColorTexture>("baseColorTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::normalTexture>("normalTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::normalMultiplier>("normalMultiplier").Attribute<UIProperty>();
        type.Field<&MaterialAsset::metallic>("metallic").Attribute<UIProperty>().Attribute<UIFloatProperty>(0.0f, 1.0f);
        type.Field<&MaterialAsset::metallicTexture>("metallicTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::roughness>("roughness").Attribute<UIProperty>().Attribute<UIFloatProperty>(0.0f, 1.0f);
        type.Field<&MaterialAsset::roughnessTexture>("roughnessTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::metallicRoughnessTexture>("metallicRoughnessTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::aoTexture>("aoTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::emissiveTexture>("emissiveTexture").Attribute<UIProperty>();
        type.Field<&MaterialAsset::emissiveFactor>("emissiveFactor").Attribute<UIProperty>();
        type.Field<&MaterialAsset::alphaCutoff>("alphaCutoff").Attribute<UIProperty>().Attribute<UIFloatProperty>(0.0f, 1.0f);
        type.Field<&MaterialAsset::alphaMode>("alphaMode").Attribute<UIProperty>();
        type.Field<&MaterialAsset::uvScale>("uvScale").Attribute<UIProperty>();
    }
}
