#include "TextureAsset.hpp"

#include <stb_image_resize.h>

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/Graphics.hpp"

namespace Fyrion
{
    void TextureAssetImage::RegisterType(NativeTypeHandler<TextureAssetImage>& type)
    {
        type.Field<&TextureAssetImage::byteOffset>("byteOffset");
        type.Field<&TextureAssetImage::mip>("mip");
        type.Field<&TextureAssetImage::arrayLayer>("arrayLayer");
        type.Field<&TextureAssetImage::extent>("extent");
        type.Field<&TextureAssetImage::size>("size");
    }

    TextureAsset::~TextureAsset()
    {
        if (texture)
        {
            Graphics::DestroyTexture(texture);
        }

        if(sampler)
        {
            Graphics::DestroySampler(sampler);
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
        mipLevels = generateMipmaps ? static_cast<u32>(std::floor(std::log2(std::max(image.GetWidth(), image.GetHeight())))) + 1 : 1;

        format = Format::RGBA;
        arrayLayers = 1;

        usize bytesSize{};
        //calc bytes
        {
            u32 mipWidth = image.GetWidth();
            u32 mipHeight = image.GetHeight();
            for (u32 i = 0; i < mipLevels; i++)
            {
                bytesSize += mipWidth * mipHeight * image.GetChannels();
                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }
        }


        Array<u8> byteImages{};
        byteImages.Resize(bytesSize);

        i32 mipWidth = image.GetWidth();
        i32 mipHeight = image.GetHeight();
        u32 offset{};

        for (u32 i = 0; i < mipLevels; i++)
        {
            images.EmplaceBack(TextureAssetImage{
                .byteOffset = offset,
                .mip = i,
                .arrayLayer = 0,
                .extent = Extent{static_cast<u32>(mipWidth), static_cast<u32>(mipHeight)},
                .size = static_cast<usize>(mipWidth * mipHeight * image.GetChannels())
            });

            memcpy(byteImages.Data() + offset, image.GetData().begin(), mipWidth * mipHeight * image.GetChannels());

            if (mipWidth > 1 && mipHeight > 1)
            {
                stbir_resize_uint8(byteImages.Data() + offset, mipWidth, mipHeight, 0, image.GetData().begin(), mipWidth / 2, mipHeight / 2, 0, image.GetChannels());
            }

            offset += mipWidth * mipHeight * image.GetChannels();

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        SaveBlob(textureData, byteImages.Data(), byteImages.Size());
    }

    void TextureAsset::SetHDRImage(const HDRImage& image)
    {
        Span<f32> imgData = image.GetData();
        SaveBlob(textureData, imgData.Data(), imgData.Size() * sizeof(f32));

        // width = image.GetWidth();
        // height = image.GetHeight();
        // channels = image.GetChannels();
        format = Format::RGBA32F;
    }

    Texture TextureAsset::CreateTexture() const
    {
        usize size = GetBlobSize(textureData);
        if (size == 0)
        {
            return {};
        }

        Texture texture = Graphics::CreateTexture(TextureCreation{
            .extent = {images[0].extent.width, images[0].extent.height, 1},
            .format = format,
            .mipLevels = std::max(mipLevels, 1u),
            .arrayLayers = std::max(arrayLayers, 1u)
        });

        Array<u8> textureBytes(size);
        LoadBlob(textureData, textureBytes.Data(), textureBytes.Size());


        Array<TextureDataRegion> regions{};
        regions.Reserve(images.Size());

        for (const TextureAssetImage& textureAssetImage : images)
        {
            regions.EmplaceBack(TextureDataRegion{
                .dataOffset = textureAssetImage.byteOffset,
                .mipLevel = textureAssetImage.mip,
                .arrayLayer = textureAssetImage.arrayLayer,
                .extent = Extent3D{textureAssetImage.extent.width, textureAssetImage.extent.height, 1},
            });
        }

        Graphics::UpdateTextureData(TextureDataInfo{
            .texture = texture,
            .data = textureBytes.Data(),
            .size = textureBytes.Size(),
            .regions = regions
        });

        return texture;
    }

    Texture TextureAsset::GetTexture()
    {
        if (!texture)
        {
            texture = CreateTexture();
            if (!texture)
            {
                return Graphics::GetDefaultTexture();
            }
        }
        return texture;
    }

    Sampler TextureAsset::GetSampler()
    {
        if (!sampler)
        {
            sampler = Graphics::CreateSampler({
                .minLod = 0,
                .maxLod = static_cast<f32>(mipLevels),
            });
        }
        return sampler;
    }

    Image TextureAsset::GetImage() const
    {
        Image image{images[0].extent.width, images[0].extent.height, 4};
        LoadBlob(textureData, image.GetData().begin(), image.GetData().Size());
        return image;
    }

    Format TextureAsset::GetFormat() const
    {
        return format;
    }

    void TextureAsset::GenerateMipmaps() {}

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::format>("format");
        type.Field<&TextureAsset::generateMipmaps>("generateMipmaps").Attribute<UIProperty>();
        type.Field<&TextureAsset::generateCubemap>("generateCubemap").Attribute<UIProperty>();

        type.Field<&TextureAsset::mipLevels>("mipLevels");
        type.Field<&TextureAsset::arrayLayers>("arrayLayers");
        type.Field<&TextureAsset::imageBuffer>("imageBuffer");
        type.Field<&TextureAsset::images>("images");
        type.Field<&TextureAsset::texture>("texture");
        type.Field<&TextureAsset::textureData>("textureData");
        type.Field<&TextureAsset::cubeMapData>("cubeMapData");
    }
}
