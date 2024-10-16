#include "TextureAsset.hpp"

#include "Fyrion/Core/Registry.hpp"
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

    void TextureAsset::SetData(u8* bytes, u32 width, u32 height, Format format)
    {
        this->format = format;
        this->extent = {width, height, 1};

        usize sizeInBytes = width * height * 4; //TODO check Format.

        OutputFileStream stream = CreateStream();
        stream.Write(bytes, sizeInBytes);
        stream.Close();
    }

    Texture TextureAsset::GetTexture()
    {
        if (!texture)
        {
            usize sizeInBytes = extent.width * extent.height * 4; //TODO check Format.

            texture = Graphics::CreateTexture(TextureCreation{
                .extent = extent,
                .format = format,
            });

            Array<u8> data = LoadStream(0, sizeInBytes);

            TextureDataRegion region{
                .extent = extent,
            };

            Graphics::UpdateTextureData(TextureDataInfo{
                .texture = texture,
                .data = data.Data(),
                .size = data.Size(),
                .regions = {&region, 1}
            });
        }
        return texture;
    }

    Image TextureAsset::GetImage() const
    {
        usize sizeInBytes = extent.width * extent.height * 4; //TODO check Format.
        Image image(extent.width, extent.height, 4);
        image.data =  LoadStream(0, sizeInBytes);
        return image;
    }

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::extent>("extent");
        type.Field<&TextureAsset::format>("format");
       // type.Field<&TextureAsset::textureData>("textureData");
    }
}
