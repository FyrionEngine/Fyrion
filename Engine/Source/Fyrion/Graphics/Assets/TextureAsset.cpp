#include "TextureAsset.hpp"

#include <stb_image_resize.h>

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
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
                ImportCubemap(image.GetWidth(), image.GetHeight(), image.GetData().Data());
                break;
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

    void TextureAsset::ImportTexture2D()
    {

    }

    void TextureAsset::ImportCubemap(u32 width, u32 height, ConstPtr bytes)
    {
        TextureImportData* textureImportData = MemoryGlobals::GetDefaultAllocator().Alloc<TextureImportData>(TextureImportData{
            .textureAsset = this,
            .width = width,
            .height = height,
            .bytes = bytes
        });

        Graphics::AddTask(GraphicsTaskType::Graphics, textureImportData, [](VoidPtr userData, RenderCommands& cmd, GPUQueue queue)
        {
            Extent cubemapSize = {1024, 1024};

            TextureImportData* textureImportData = static_cast<TextureImportData*>(userData);
            TextureAsset* textureAsset = textureImportData->textureAsset;

            BufferCreation vertexBufferCreation{
                .usage = BufferUsage::VertexBuffer,
                .size = sizeof(cubemapVertices),
                .allocation = BufferAllocation::TransferToGPU
            };

            Buffer renderBuffer = Graphics::CreateBuffer(vertexBufferCreation);

            BufferDataInfo vertexDataInfo{
                .buffer = renderBuffer,
                .data = cubemapVertices,
                .size = sizeof(cubemapVertices)
            };
            Graphics::UpdateBufferData(vertexDataInfo);

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

            Texture passTexture = Graphics::CreateTexture(TextureCreation{
                .extent = {cubemapSize.width, cubemapSize.height, 1},
                .format = textureAsset->GetFormat(),
                .usage = TextureUsage::RenderPass | TextureUsage::TransferSrc
            });

            AttachmentCreation attachmentCreation{
                .texture = passTexture,
                .finalLayout = ResourceLayout::ColorAttachment
            };

            RenderPass renderPass = Graphics::CreateRenderPass(RenderPassCreation{
                .attachments = &attachmentCreation
            });

            ShaderAsset* shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Utils/EquirectangularToCubemap.raster");

            PipelineState pipelineState = Graphics::CreateGraphicsPipelineState(GraphicsPipelineCreation{
                .shader = shader,
                .renderPass = renderPass,
            });

            BindingSet* bindingSet = Graphics::CreateBindingSet(shader);
            bindingSet->GetVar("texture")->SetTexture(texture);

            Mat4 captureProjection = Math::Perspective(Math::Radians(90.0f), 1.0f, 0.1f, 10.0f);

            Mat4 captureViews[] =
            {
                Math::LookAt(Vec3{0.0f, 0.0f, 0.0f}, Vec3{-1.0f, 0.0f, 0.0f}, Vec3{0.0f, -1.0f, 0.0f}),
                Math::LookAt(Vec3{0.0f, 0.0f, 0.0f}, Vec3{1.0f, 0.0f, 0.0f}, Vec3{0.0f, -1.0f, 0.0f}),
                Math::LookAt(Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f}, Vec3{0.0f, 0.0f, 1.0f}),
                Math::LookAt(Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, -1.0f, 0.0f}, Vec3{0.0f, 0.0f, -1.0f}),
                Math::LookAt(Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, 0.0f, 1.0f}, Vec3{0.0f, -1.0f, 0.0f}),
                Math::LookAt(Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, 0.0f, -1.0f}, Vec3{0.0f, -1.0f, 0.0})
            };

            Array<u8> textureBytes{};
            Array<u8> byteImages{};
            byteImages.Reserve(cubemapSize.width * cubemapSize.height * GetFormatSize(textureAsset->GetFormat()));

            for (int i = 0; i < 6; ++i)
            {
                Mat4 projView = captureProjection * captureViews[i];

                cmd.Begin();

                Vec4 color = {0, 0, 0, 1};

                cmd.BeginRenderPass(BeginRenderPassInfo{
                    .renderPass = renderPass,
                    .clearValue = &color
                });

                ViewportInfo viewportInfo{
                    .x = 0,
                    .y = (f32)cubemapSize.height,
                    .width = (f32)cubemapSize.width,
                    .height = -(f32)cubemapSize.height
                };

                cmd.SetViewport(viewportInfo);
                cmd.SetScissor(Rect{0, 0, cubemapSize.width, cubemapSize.height});

                cmd.BindPipelineState(pipelineState);
                cmd.BindVertexBuffer(renderBuffer);

                cmd.PushConstants(pipelineState, ShaderStage::Vertex, &projView, sizeof(Mat4));
                cmd.BindBindingSet(pipelineState, bindingSet);

                cmd.Draw(36, 1, 0, 0);

                cmd.EndRenderPass();

                cmd.ResourceBarrier(ResourceBarrierInfo{
                    .texture = passTexture,
                    .oldLayout = ResourceLayout::ColorAttachment,
                    .newLayout = ResourceLayout::CopySource
                });

                cmd.SubmitAndWait(queue);

                TextureGetDataInfo info{
                    .texture = passTexture,
                    .format = textureAsset->GetFormat(),
                    .extent = {cubemapSize.height, cubemapSize.width},
                    .textureLayout = ResourceLayout::CopySource,
                };

                Graphics::GetTextureData(info, textureBytes);

                textureAsset->images.EmplaceBack(TextureAssetImage{
                	.byteOffset = static_cast<u32>(byteImages.Size()),
                	.arrayLayer = static_cast<u32>(i),
                	.extent = cubemapSize,
                	.size = textureBytes.Size()
                });

                byteImages.Insert(byteImages.end(), textureBytes.begin(), textureBytes.end());
            }

            textureAsset->arrayLayers = 6;
            textureAsset->mipLevels = 1;
            textureAsset->SaveBlob(textureAsset->textureData, byteImages.Data(), byteImages.Size());

            Graphics::DestroyRenderPass(renderPass);
            Graphics::DestroyGraphicsPipelineState(pipelineState);
            Graphics::DestroyTexture(passTexture);
            Graphics::DestroyBuffer(renderBuffer);
            Graphics::DestroyBindingSet(bindingSet);
            Graphics::DestroyTexture(texture);

            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(textureImportData);
        });
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
