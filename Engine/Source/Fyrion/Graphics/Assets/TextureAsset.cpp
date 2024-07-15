#include "TextureAsset.hpp"

#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/Graphics.hpp"

namespace Fyrion
{
    TextureAsset::~TextureAsset()
    {
        if (texture)
        {
            Graphics::DestroyTexture(texture);
        }
    }

    StringView TextureAsset::GetDisplayName() const
    {
        return "Texture";
    }

    void TextureAsset::SetImage(StringView path)
    {
        Image    image(path);
        Span<u8> imgData = image.GetData();

        SaveBlob(data, imgData.Data(), imgData.Size());

        width = image.GetWidth();
        height = image.GetHeight();
        channels = image.GetChannels();
    }

    Texture TextureAsset::GetTexture()
    {
        if (!texture)
        {
            texture = Graphics::CreateTexture(TextureCreation{
                .extent = {width, height, 1},
            });

            usize     size = GetBlobSize(data);
            Array<u8> textureData(size);
            LoadBlob(data, textureData.Data(), textureData.Size());

            Graphics::UpdateTextureData(TextureDataInfo{
                .texture = texture,
                .data = textureData.Data(),
                .size = size,
                .extent = {width, height, 1}
            });
        }

        return texture;
    }

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::width>("width");
        type.Field<&TextureAsset::height>("height");
        type.Field<&TextureAsset::channels>("channels");
        type.Field<&TextureAsset::data>("data");
    }
}
