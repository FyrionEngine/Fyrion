#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    enum class ShaderAssetType
    {
        None,
        Graphics,
        Compute,
        Raytrace
    };

    class FY_API ShaderAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        StringView GetDisplayName() const override;

        StringView GetShaderSource() const
        {
            return shaderSource;
        }

        ShaderAssetType GetShaderType() const
        {
            return shaderType;
        }

        void SetShaderSource(const String& shaderSource)
        {
            this->shaderSource = shaderSource;
        }

        void SetShaderType(ShaderAssetType shaderType)
        {
            this->shaderType = shaderType;
        }

        const ShaderInfo& GetShaderInfo() const
        {
            return shaderInfo;
        }

        Span<ShaderStageInfo> GetStages() const;
        Span<u8>              GetBytes() const;

        void Compile();

        static void RegisterType(NativeTypeHandler<ShaderAsset>& type);

    private:
        String                 shaderSource;
        ShaderAssetType        shaderType = ShaderAssetType::None;
        Array<u8>              bytes{};
        Array<ShaderStageInfo> stages{};
        ShaderInfo             shaderInfo{};
    };
}
