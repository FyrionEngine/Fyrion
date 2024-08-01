#include "PanoramicSky.hpp"

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/Scene/SceneObject.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"


namespace Fyrion
{
    void PanoramicSky::OnChange()
    {
        if (object->IsActivated())
        {
            if (sphericalTexture != nullptr)
            {
                //sphericalTexture->LoadBlob();
            }
        }
    }

    void PanoramicSky::SetSphericalTexture(TextureAsset* sphericalTexture)
    {
        this->sphericalTexture = sphericalTexture;
        OnChange();
    }

    TextureAsset* PanoramicSky::GetSphericalTexture() const
    {
        return sphericalTexture;
    }

    void PanoramicSky::RegisterType(NativeTypeHandler<PanoramicSky>& type)
    {
        type.Field<&PanoramicSky::sphericalTexture>("sphericalTexture").Attribute<UIProperty>();
    }

    void PanoramicSky::GenerateCubemap()
    {
        Graphics::AddTask(GraphicsTaskType::Graphics, this, [](VoidPtr userData, RenderCommands& cmd, GPUQueue queue)
        {
            PanoramicSky* panoramicSky = static_cast<PanoramicSky*>(userData);
            Extent        cubemapSize = {2048, 2048};

            TextureAsset* textureAsset = panoramicSky->GetSphericalTexture();

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

            Texture texture = textureAsset->GetTexture();

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

                // TextureGetDataInfo info{
                // 	.texture =  passTexture,
                // 	.textureLayout = ResourceLayout::CopySource,
                // };
                //
                // Graphics::GetTextureData(cmd, info, textureBytes);
                //
                // textureAsset.images.EmplaceBack(TextureAssetImage{
                // 	.byteOffset = (u32)byteImages.Size(),
                // 	.arrayLayer = (u32)i,
                // 	.extent = cubemapSize,
                // 	.size = textureBytes.Size()
                // });

                byteImages.Insert(byteImages.end(), textureBytes.begin(), textureBytes.end());
            }

            // textureAsset.format = format;
            // textureAsset.arrayLayers = 6;
            // textureAsset.mipLevels = 1;

            // ResourceServer::SetDirty(rid);
            // ResourceServer::Set(resourceAssetRid, &textureAsset);
            // ResourceServer::CreateBuffer(rid, textureAsset.imageBuffer, byteImages.Data(), byteImages.Size());

            Graphics::DestroyRenderPass(renderPass);
            Graphics::DestroyGraphicsPipelineState(pipelineState);
            Graphics::DestroyTexture(passTexture);
            Graphics::DestroyBuffer(renderBuffer);
            Graphics::DestroyBindingSet(bindingSet);
            Graphics::DestroyTexture(texture);
        });
    }
}
