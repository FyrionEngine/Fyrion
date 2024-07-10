#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"

namespace Fyrion
{
    enum class ShaderAssetType
    {
        None,
        Raster,
        Compute,
        Raytrace
    };

    struct FY_API ShaderIO : public AssetIO
    {
        StringView extension[1] = {".hlsl"};

        FY_BASE_TYPES(AssetIO);

        Span<StringView> GetImportExtensions() override;
        Asset*           ImportAsset(StringView path, Asset* reimportAsset) override;
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
