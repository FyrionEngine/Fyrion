#include "TextureAsset.hpp"

#include <stb_image_resize.h>

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderUtils.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion
{
    struct TextureImportData
    {
        TextureAsset* textureAsset;
        u32           width;
        u32           height;
        ConstPtr      bytes;
    };


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

        if (sampler)
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
        images.Clear();

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
        format = Format::RGBA32F;
        images.Clear();

        switch (textureType)
        {
            case TextureType::Texture2D:
            {
                arrayLayers = 1;

                images.EmplaceBack(TextureAssetImage{
                    .byteOffset = 0,
                    .mip = 0,
                    .arrayLayer = 0,
                    .extent = Extent{image.GetWidth(), image.GetHeight()},
                    .size = static_cast<usize>(image.GetWidth() * image.GetHeight() * image.GetChannels())
                });

                SaveBlob(textureData, image.GetData().Data(), image.GetData().Size() * sizeof(f32));

                break;
            }
            case TextureType::Texture3D:
                break;
            case TextureType::Cubemap:
            {
                TextureImportData* textureImportData = MemoryGlobals::GetDefaultAllocator().Alloc<TextureImportData>(TextureImportData{
                    .textureAsset = this,
                    .width = image.GetWidth(),
                    .height = image.GetHeight(),
                    .bytes = image.GetData().Data()
                });

                Graphics::AddTask(GraphicsTaskType::Graphics, textureImportData, [](VoidPtr userData, RenderCommands& cmd, GPUQueue queue)
                {
                    TextureImportData* textureImportData = static_cast<TextureImportData*>(userData);
                    TextureAsset* textureAsset = textureImportData->textureAsset;

                    Texture texture = Graphics::CreateTexture(TextureCreation{
                        .extent = {textureImportData->width, textureImportData->height, 1},
                        .format = textureAsset->GetFormat(),
                    });

                    TextureDataRegion region{
                        .extent = {textureImportData->width, textureImportData->height, 1}
                    };

                    Graphics::UpdateTextureData(TextureDataInfo{
                        .texture = texture,
                        .data = static_cast<const u8*>(textureImportData->bytes),
                        .size = textureImportData->width * textureImportData->height * GetFormatSize(textureAsset->GetFormat()),
                        .regions = {&region, 1}
                    });

                    Texture cubemap = RenderUtils::ConvertEquirectangularToCubemap(cmd, queue, texture, Format::RGBA16F, {1024, 1024});

                    // Array<u8> bytes;
                    // Graphics::GetTextureData(TextureGetDataInfo{
                    //     .texture = cubemap,
                    //     .extent =
                    // },
                    // bytes);
                });

                break;
            }

        }
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

    TextureType TextureAsset::GetTextureType() const
    {
        return textureType;
    }

    void TextureAsset::SetTextureType(TextureType textureType)
    {
        this->textureType = textureType;
    }

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::format>("format");
        type.Field<&TextureAsset::textureType>("textureType").Attribute<UIProperty>();
        type.Field<&TextureAsset::generateMipmaps>("generateMipmaps").Attribute<UIProperty>();

        type.Field<&TextureAsset::mipLevels>("mipLevels");
        type.Field<&TextureAsset::arrayLayers>("arrayLayers");
        type.Field<&TextureAsset::imageBuffer>("imageBuffer");
        type.Field<&TextureAsset::images>("images");
        type.Field<&TextureAsset::textureData>("textureData");
    }
}
