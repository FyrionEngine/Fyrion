#include <stb_image.h>

#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"
#include "Fyrion/Editor/Window/TextureViewWindow.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct TextureAssetHandler : JsonAssetHandler
    {
        FY_BASE_TYPES(JsonAssetHandler);

        StringView Extension() override
        {
            return ".texture";
        }

        TypeID GetAssetTypeID() override
        {
            return GetTypeID<TextureAsset>();
        }

        void OpenAsset(AssetFile* assetFile) override
        {
            TextureAsset* textureAsset = Assets::Load<TextureAsset>(assetFile->uuid);
            TextureViewWindow::Open(textureAsset->GetTexture());
        }
    };


    struct TextureImporter : AssetImporter
    {
        FY_BASE_TYPES(AssetImporter);

        Array<String> ImportExtensions() override
        {
            return {".png", ".jpg", ".jpeg", ".tga", "bmp", ".hdr"};
        }

        void ImportAsset(AssetFile* parent, StringView path) override
        {
            AssetFile* assetFile = AssetEditor::CreateAsset(parent, GetTypeID<TextureAsset>(), Path::Name(path));

            TextureAsset* textureAsset = Assets::Load<TextureAsset>(assetFile->uuid);

            i32 imageWidth{};
            i32 imageHeight{};
            i32 imageChannels{};
            u8* bytes = stbi_load(path.CStr(), &imageWidth, &imageHeight, &imageChannels, 4);

            textureAsset->SetData(bytes, imageWidth, imageHeight, Format::RGBA);

            stbi_image_free(bytes);
        }
    };


    void RegisterTextureAssetHandler()
    {
        Registry::Type<TextureImporter>();
        Registry::Type<TextureAssetHandler>();
    }
}
