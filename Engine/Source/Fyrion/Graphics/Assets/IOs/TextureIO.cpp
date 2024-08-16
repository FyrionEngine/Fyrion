#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct FY_API TextureIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        TextureIO()
        {
            getImportExtensions = GetImportExtensions;
            getAssetTypeId = GetAssetTypeID;
            importAsset = ImportAsset;
        }

        static inline StringView extension[6] = {".png", ".jpg", ".jpeg", ".tga", "bmp", ".hdr"};

        static Span<StringView> GetImportExtensions()
        {
            return {extension, 6};
        }

        static TypeID GetAssetTypeID(StringView path)
        {
            return GetTypeID<TextureAsset>();
        }

        static void ImportAsset(StringView path, Asset* asset)
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
