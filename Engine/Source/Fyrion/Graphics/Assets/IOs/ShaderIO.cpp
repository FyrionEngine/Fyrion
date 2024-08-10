#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct FY_API ShaderIO : public AssetIO
    {
        StringView extension[4] = {".raster", ".comp", ".inc", ".hlsl"};

        FY_BASE_TYPES(AssetIO);

        Span<StringView> GetImportExtensions() override
        {
            return {extension, sizeof(extension)/sizeof(StringView)};
        }

        TypeID GetAssetTypeID(StringView path) override
        {
            return GetTypeID<ShaderAsset>();
        }

        void ImportAsset(StringView path, Asset* asset) override
        {
            ShaderAsset* shaderAsset = asset->Cast<ShaderAsset>();
            shaderAsset->SetShaderSource(FileSystem::ReadFileAsString(path));

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

            //recompile shader.
            if (shaderAsset->IsCompiled())
            {
                shaderAsset->Compile();
            }
        }
    };

    void RegisterShaderIO()
    {
        Registry::Type<ShaderIO>();
    }
}
