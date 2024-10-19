#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion
{
    class ShaderAssetHandler : public AssetHandler
    {
    public:
        FY_BASE_TYPES(AssetHandler);

        TypeID GetAssetTypeID() override
        {
            return GetTypeID<ShaderAsset>();
        }

        void Save(StringView newPath, AssetFile* assetFile) override
        {

        }

        void Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance) override
        {
            //static_cast<ShaderAsset*>(instance);
        }

        void OpenAsset(AssetFile* assetFile) override
        {
            Assets::Load<ShaderAsset>(assetFile->uuid);
        }

        Image GenerateThumbnail(AssetFile* assetFile) override
        {
            return Image{};
        }
    };

    class RasterShaderAssetHandler : public ShaderAssetHandler
    {
    public:
        FY_BASE_TYPES(ShaderAssetHandler);

        StringView Extension() override
        {
            return ".raster";
        }
    };

    class ComputeShaderAssetHandler : public ShaderAssetHandler
    {
    public:
        FY_BASE_TYPES(ShaderAssetHandler);

        StringView Extension() override
        {
            return ".compute";
        }
    };


    void RegisterShaderAssetHandlers()
    {
        Registry::Type<ShaderAssetHandler>();
        Registry::Type<RasterShaderAssetHandler>();
        Registry::Type<ComputeShaderAssetHandler>();
    }
}
