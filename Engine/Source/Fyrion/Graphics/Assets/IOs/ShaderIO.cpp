#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct FY_API ShaderIO : public AssetIO
    {
        static inline StringView extension[4] = {".raster", ".comp", ".inc", ".hlsl"};

        FY_BASE_TYPES(AssetIO);

        ShaderIO()
        {
            getImportExtensions = GetImportExtensions;
            getAssetTypeId = GetAssetTypeID;
            importAsset = ImportAsset;
        }

        static Span<StringView> GetImportExtensions()
        {
            return {extension, sizeof(extension)/sizeof(StringView)};
        }

        static TypeID GetAssetTypeID(StringView path)
        {
            return GetTypeID<ShaderAsset>();
        }

        static void ImportAsset(StringView path, Asset* asset)
        {
            ShaderAsset* shaderAsset = asset->Cast<ShaderAsset>();

            if (Path::Extension(path) == ".raster")
            {
                shaderAsset->SetShaderType(ShaderAssetType::Graphics);
            }
            else if (Path::Extension(path) == ".comp")
            {
                shaderAsset->SetShaderType(ShaderAssetType::Compute);
            }
            else if (Path::Extension(path) == ".inc" || Path::Extension(path) == ".hlsl")
            {
                shaderAsset->SetShaderType(ShaderAssetType::Include);
            }

            shaderAsset->Compile();
        }
    };

    void RegisterShaderIO()
    {
        Registry::Type<ShaderIO>();
    }
}
