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

    void TextureAsset::SetImagePath(StringView path)
    {
        Image image(path);
        SetImage(image);
    }

    void TextureAsset::SetHDRImagePath(StringView path)
    {
        HDRImage image(path);
        SetHDRImage(image);
    }

    void TextureAsset::SetImage(const Image& image)
    {
        Span<u8> imgData = image.GetData();

        SaveBlob(data, imgData.Data(), imgData.Size());

        width = image.GetWidth();
        height = image.GetHeight();
        channels = image.GetChannels();
        format = Format::RGBA;
    }

    void TextureAsset::SetHDRImage(const HDRImage& image)
    {
        Span<f32> imgData = image.GetData();
        SaveBlob(data, imgData.Data(), imgData.Size() * sizeof(f32));

        width = image.GetWidth();
        height = image.GetHeight();
        channels = image.GetChannels();
        format = Format::RGBA32F;
    }

    Texture TextureAsset::GetTexture()
    {
        if (!texture)
        {
            usize size = GetBlobSize(data);
            if (size == 0)
            {
                return Graphics::GetDefaultTexture();
            }

            texture = Graphics::CreateTexture(TextureCreation{
                .extent = {width, height, 1},
            });

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

    Image TextureAsset::GetImage() const
    {
        Image image{width, height, channels};
        LoadBlob(data, image.GetData().begin(), image.GetData().Size());
        return image;
    }

    Format TextureAsset::GetFormat() const
    {
        return format;
    }

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::width>("width");
        type.Field<&TextureAsset::height>("height");
        type.Field<&TextureAsset::channels>("channels");
        type.Field<&TextureAsset::format>("format");
        type.Field<&TextureAsset::data>("data");
    }
}
