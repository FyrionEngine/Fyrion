#include <stb_image.h>

#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"
#include "Fyrion/Editor/Window/TextureViewWindow.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/IO/Path.hpp"
#include "TextureAssetHandler.hpp"

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

        Image GenerateThumbnail(AssetFile* assetFile) override
        {
            TextureAsset* textureAsset = Assets::Load<TextureAsset>(assetFile->uuid);
            Image image = textureAsset->GetImage();
            image.Resize(128, 128);
            return image;
        }
    };


    struct TextureAssetImporter : AssetImporter
    {
        FY_BASE_TYPES(AssetImporter);

        Array<String> ImportExtensions() override
        {
            return {".png", ".jpg", ".jpeg", ".tga", "bmp", ".hdr"};
        }

        bool ImportAsset(AssetFile* parent, StringView path) override
        {
            AssetFile* assetFile = AssetEditor::CreateAsset(parent, GetTypeID<TextureAsset>(), Path::Name(path));

            TextureAsset* textureAsset = Assets::Load<TextureAsset>(assetFile->uuid);

            i32 imageWidth{};
            i32 imageHeight{};
            i32 imageChannels{};
            u8* bytes = stbi_load(path.CStr(), &imageWidth, &imageHeight, &imageChannels, 4);

            usize sizeInBytes = imageWidth * imageHeight * 4; //TODO check Format.

            OutputFileStream stream = assetFile->CreateStream();
            stream.Write(bytes, sizeInBytes);
            stream.Close();

            textureAsset->SetProperties(imageWidth, imageHeight, Format::RGBA);

            stbi_image_free(bytes);

            return true;
        }
    };


    void RegisterTextureAssetHandler()
    {
        Registry::Type<TextureAssetImporter>();
        Registry::Type<TextureAssetHandler>();
    }

    void TextureImporter::ImportTexture(AssetFile* assetFile, TextureAsset* textureAsset, Span<const u8> imageBuffer)
    {
        i32 imageWidth{};
        i32 imageHeight{};
        i32 imageChannels{};
        u8* bytes = stbi_load_from_memory(imageBuffer.Data(), imageBuffer.Size(), &imageWidth, &imageHeight, &imageChannels, 4);

        usize sizeInBytes = imageWidth * imageHeight * 4; //TODO check Format.

        OutputFileStream stream = assetFile->CreateStream();
        stream.Write(bytes, sizeInBytes);
        stream.Close();

        textureAsset->SetProperties(imageWidth, imageHeight, Format::RGBA);

        stbi_image_free(bytes);
    }
}
