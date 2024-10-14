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
        this->textureData.Store({bytes, sizeInBytes});
    }

    Texture TextureAsset::GetTexture()
    {
        if (!texture)
        {
            texture = Graphics::CreateTexture(TextureCreation{
                .extent = extent,
                .format = format,
            });

            Array<u8> data = textureData.Load();

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

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::extent>("extent");
        type.Field<&TextureAsset::format>("format");
        type.Field<&TextureAsset::textureData>("textureData");
    }
}
