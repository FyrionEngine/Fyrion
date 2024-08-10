#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct FY_API TextureIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        StringView extension[6] = {".png", ".jpg", ".jpeg", ".tga", "bmp", ".hdr"};

        Span<StringView> GetImportExtensions() override
        {
            return {extension, 6};
        }

        TypeID GetAssetTypeID(StringView path) override
        {
            return GetTypeID<TextureAsset>();
        }

        void ImportAsset(StringView path, Asset* asset) override
        {
            TextureAsset* textureAsset = asset->Cast<TextureAsset>();

            if (Path::Extension(path) == ".hdr")
            {
                textureAsset->SetHDRImage(path);
            }
            else
            {
                textureAsset->SetImagePath(path);
            }
        }
    };

    void RegisterTextureIO()
    {
        Registry::Type<TextureIO>();
    }
}
