#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct FY_API ShaderIO : public AssetIO
    {
        StringView extension[3] = {".raster", ".comp", ".inc"};

        FY_BASE_TYPES(AssetIO);

        Span<StringView> GetImportExtensions() override
        {
            return {extension, 3};
        }

        Asset* CreateAsset() override
        {
            return AssetDatabase::Create<ShaderAsset>();
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
            else if (Path::Extension(path) == ".inc")
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
