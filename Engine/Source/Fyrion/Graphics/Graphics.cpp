#include "Graphics.hpp"

#include <mutex>

#include "Fyrion/Core/FixedArray.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/Device/RenderDevice.hpp"
#include "Fyrion/Core/SharedPtr.hpp"

namespace Fyrion
{
    void GraphicsInit();
    void GraphicsShutdown();
    void GraphicsCreateDevice(Adapter adapter);

    SharedPtr<RenderDevice> CreateVulkanDevice();

    namespace
    {
        SharedPtr<RenderDevice> renderDevice = {};
        Sampler                 defaultSampler = {};
        Texture                 defaultTexture = {};
        std::mutex mutex;
    }

    void GraphicsInit()
    {
        renderDevice = CreateVulkanDevice();
    }

    void GraphicsShutdown()
    {
        renderDevice->DestroySampler(defaultSampler);
        renderDevice->DestroyTexture(defaultTexture);
        renderDevice = {};
    }

    void GraphicsCreateDevice(Adapter adapter)
    {
        renderDevice->CreateDevice(adapter);

        defaultSampler = renderDevice->CreateSampler({
            .maxLod = 200
        });

        defaultTexture = renderDevice->CreateTexture({
            .extent = {1, 1, 1}
        });

        Image image(1, 1, 4);
        image.SetPixelColor(0, 0, Color::WHITE);

        TextureDataRegion region{
            .extent = {1, 1, 1}
        };

        Graphics::UpdateTextureData(TextureDataInfo{
            .texture = defaultTexture,
            .data = image.GetData().Data(),
            .size = image.GetData().Size(),
            .regions = {&region, 1}
        });
    }

    RenderCommands& GraphicsBeginFrame()
    {
        return renderDevice->BeginFrame();
    }

    RenderDevice& GetRenderDevice()
    {
        return *renderDevice;
    }

    void GraphicsEndFrame(Swapchain swapchain)
    {
        renderDevice->EndFrame(swapchain);
    }

    Span<Adapter> Graphics::GetAdapters()
    {
        return renderDevice->GetAdapters();
    }

    Swapchain Graphics::CreateSwapchain(const SwapchainCreation& swapchainCreation)
    {
        return renderDevice->CreateSwapchain(swapchainCreation);
    }

    RenderPass Graphics::CreateRenderPass(const RenderPassCreation& renderPassCreation)
    {
        return renderDevice->CreateRenderPass(renderPassCreation);
    }

    Buffer Graphics::CreateBuffer(const BufferCreation& bufferCreation)
    {
        return renderDevice->CreateBuffer(bufferCreation);
    }

    Texture Graphics::CreateTexture(const TextureCreation& textureCreation)
    {
        return renderDevice->CreateTexture(textureCreation);
    }

    Texture Graphics::CreateTextureFromImage(const Image& image)
    {
        Texture texture = CreateTexture(TextureCreation{
            .extent = {image.GetWidth(), image.GetHeight(), 1},
            .format = Format::RGBA,
        });

        TextureDataRegion region{
            .extent = {image.GetWidth(), image.GetHeight(), 1},
        };

        UpdateTextureData(TextureDataInfo{
            .texture = texture,
            .data = image.GetData().Data(),
            .size = image.GetData().Size(),
            .regions = {&region, 1}
        });

        return texture;
    }

    TextureView Graphics::CreateTextureView(const TextureViewCreation& textureViewCreation)
    {
        return renderDevice->CreateTextureView(textureViewCreation);
    }

    Sampler Graphics::CreateSampler(const SamplerCreation& samplerCreation)
    {
        return renderDevice->CreateSampler(samplerCreation);
    }

    PipelineState Graphics::CreateGraphicsPipelineState(const GraphicsPipelineCreation& graphicsPipelineCreation)
    {
        return renderDevice->CreateGraphicsPipelineState(graphicsPipelineCreation);
    }

    PipelineState Graphics::CreateComputePipelineState(const ComputePipelineCreation& computePipelineCreation)
    {
        return renderDevice->CreateComputePipelineState(computePipelineCreation);
    }

    BindingSet* Graphics::CreateBindingSet(ShaderAsset* shaderAsset)
    {
        return renderDevice->CreateBindingSet(shaderAsset);
    }

    BindingSet* Graphics::CreateBindingSet(Span<DescriptorLayout> descriptorLayouts)
    {
        return renderDevice->CreateBindingSet(descriptorLayouts);
    }

    void Graphics::DestroySwapchain(const Swapchain& swapchain)
    {
        renderDevice->DestroySwapchain(swapchain);
    }

    void Graphics::DestroyRenderPass(const RenderPass& renderPass)
    {
        renderDevice->DestroyRenderPass(renderPass);
    }

    void Graphics::DestroyBuffer(const Buffer& buffer)
    {
        renderDevice->DestroyBuffer(buffer);
    }

    void Graphics::DestroyTexture(const Texture& texture)
    {
        renderDevice->DestroyTexture(texture);
    }

    void Graphics::DestroyTextureView(const TextureView& textureView)
    {
        renderDevice->DestroyTextureView(textureView);
    }

    void Graphics::DestroySampler(const Sampler& sampler)
    {
        renderDevice->DestroySampler(sampler);
    }

    void Graphics::DestroyGraphicsPipelineState(const PipelineState& pipelineState)
    {
        renderDevice->DestroyGraphicsPipelineState(pipelineState);
    }

    void Graphics::DestroyComputePipelineState(const PipelineState& pipelineState)
    {
        renderDevice->DestroyComputePipelineState(pipelineState);
    }

    void Graphics::DestroyBindingSet(BindingSet* bindingSet)
    {
        renderDevice->DestroyBindingSet(bindingSet);
    }

    RenderPass Graphics::AcquireNextRenderPass(Swapchain swapchain)
    {
        return renderDevice->AcquireNextRenderPass(swapchain);
    }

    void Graphics::WaitQueue()
    {
        renderDevice->WaitQueue();
    }

    void Graphics::UpdateBufferData(const BufferDataInfo& bufferDataInfo)
    {
        renderDevice->UpdateBufferData(bufferDataInfo);
    }

    void Graphics::UpdateTextureData(const TextureDataInfo& textureDataInfo)
    {
        Buffer buffer = CreateBuffer(BufferCreation{
            .size = textureDataInfo.size,
            .allocation = BufferAllocation::TransferToGPU
        });

        MemCopy(GetBufferMappedMemory(buffer), textureDataInfo.data, textureDataInfo.size);

        RenderCommands& tempCmd = renderDevice->GetTempCmd();
        tempCmd.Begin();

        for (const TextureDataRegion& textureRegion : textureDataInfo.regions)
        {
            tempCmd.ResourceBarrier(ResourceBarrierInfo{
                .texture = textureDataInfo.texture,
                .oldLayout = ResourceLayout::Undefined,
                .newLayout = ResourceLayout::CopyDest,
                .mipLevel = textureRegion.mipLevel,
                .levelCount = Math::Max(textureRegion.levelCount, 1u),
                .baseArrayLayer = textureRegion.arrayLayer,
                .layerCount = Math::Max(textureRegion.layerCount, 1u)
            });

            BufferImageCopy region{
                .bufferOffset = textureRegion.dataOffset,
                .textureMipLevel = textureRegion.mipLevel,
                .textureArrayLayer = textureRegion.arrayLayer,
                .layerCount = Math::Max(textureRegion.layerCount, 1u),
                .imageOffset = {},
                .imageExtent = {textureRegion.extent.width, textureRegion.extent.height, Math::Max(textureRegion.extent.depth, 1u)},
            };

            tempCmd.CopyBufferToTexture(buffer, textureDataInfo.texture, {&region, 1});

            tempCmd.ResourceBarrier(ResourceBarrierInfo{
                .texture = textureDataInfo.texture,
                .oldLayout = ResourceLayout::CopyDest,
                .newLayout = ResourceLayout::ShaderReadOnly,
                .mipLevel = textureRegion.mipLevel,
                .levelCount = Math::Max(textureRegion.levelCount, 1u),
                .baseArrayLayer = textureRegion.arrayLayer,
                .layerCount = Math::Max(textureRegion.layerCount, 1u)
            });
        }

        tempCmd.SubmitAndWait(renderDevice->GetMainQueue());

        DestroyBuffer(buffer);
    }

    void Graphics::UpdateTextureLayout(Texture texture, ResourceLayout oldLayout, ResourceLayout newLayout)
    {
        RenderCommands& tempCmd = renderDevice->GetTempCmd();
        tempCmd.Begin();

        TextureCreation creation = GetTextureCreationInfo(texture);

        for (u32 m = 0; m < creation.mipLevels; ++m)
        {
            for (u32 a = 0; a < creation.arrayLayers; ++a)
            {
                tempCmd.ResourceBarrier(ResourceBarrierInfo{
                    .texture = texture,
                    .oldLayout = oldLayout,
                    .newLayout = newLayout,
                    .mipLevel = m,
                    .levelCount = 1,
                    .baseArrayLayer = a,
                    .layerCount = 1
                });
            }
        }

        tempCmd.SubmitAndWait(renderDevice->GetMainQueue());
    }

    VoidPtr Graphics::GetBufferMappedMemory(const Buffer& buffer)
    {
        return renderDevice->GetBufferMappedMemory(buffer);
    }

    void Graphics::GetTextureData(const TextureGetDataInfo& info, Array<u8>& data)
    {
        RenderCommands& tempCmd = renderDevice->GetTempCmd();
        usize           size = info.extent.width * info.extent.height * GetFormatSize(info.format);
        data.Resize(size);

        Buffer buffer = CreateBuffer(BufferCreation{
            .size = size,
            .allocation = BufferAllocation::TransferToCPU
        });

        BufferImageCopy region{
            .imageExtent = {info.extent.width, info.extent.height, 1}
        };

        tempCmd.Begin();
        tempCmd.CopyTextureToBuffer(info.texture, info.textureLayout, buffer, {&region, 1});
        tempCmd.SubmitAndWait(renderDevice->GetMainQueue());

        MemCopy(data.Data(), GetBufferMappedMemory(buffer), size);

        DestroyBuffer(buffer);
    }

    TextureCreation Graphics::GetTextureCreationInfo(Texture texture)
    {
        return renderDevice->GetTextureCreationInfo(texture);
    }

    RenderApiType Graphics::GetRenderApi()
    {
        return RenderApiType::Vulkan;
    }

    Sampler Graphics::GetDefaultSampler()
    {
        return defaultSampler;
    }

    Texture Graphics::GetDefaultTexture()
    {
        return defaultTexture;
    }

    RenderCommands& Graphics::GetCmd()
    {
        return renderDevice->GetTempCmd();
    }

    GPUQueue Graphics::GetMainQueue()
    {
        return renderDevice->GetMainQueue();
    }

    void Graphics::AddTask(GraphicsTaskType graphicsTask, VoidPtr userData, FnGraphicsTask task)
    {
        //TODO multithread enqueue.
        //TODO choose queue by GraphicsTaskType
        RenderCommands& tempCmd = renderDevice->GetTempCmd();
        task(userData, tempCmd, renderDevice->GetMainQueue());
    }
}
