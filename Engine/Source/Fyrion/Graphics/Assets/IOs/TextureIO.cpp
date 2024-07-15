#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"

namespace Fyrion
{
    struct FY_API TextureIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        StringView extension[5] = {".png", ".jpg", ".jpeg", ".tga", "bmp"};

        Span<StringView> GetImportExtensions() override
        {
            return {extension, 5};
        }

        Asset* CreateAsset() override
        {
            return AssetDatabase::Create<TextureAsset>(UUID::RandomUUID());
        }

        void ImportAsset(StringView path, Asset* asset) override
        {
            TextureAsset* textureAsset = asset->Cast<TextureAsset>();
            textureAsset->SetImage(path);
        }
    };

    void RegisterTextureIO()
    {
        Registry::Type<TextureIO>();
    }
}
