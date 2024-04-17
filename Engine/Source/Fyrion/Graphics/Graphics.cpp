#include "Graphics.hpp"
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
    }

    void GraphicsInit()
    {
        renderDevice = CreateVulkanDevice();
    }

    void GraphicsShutdown()
    {
        renderDevice = {};
    }

    void GraphicsCreateDevice(Adapter adapter)
    {
        renderDevice->CreateDevice(adapter);
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

    Buffer Graphics::CreateBuffer(const BufferCreation& bufferCreation)
    {
        return renderDevice->CreateBuffer(bufferCreation);
    }

    Texture Graphics::CreateTexture(const TextureCreation& textureCreation)
    {
        return renderDevice->CreateTexture(textureCreation);
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

    BindingSet& Graphics::CreateBindingSet(RID shader)
    {
        return renderDevice->CreateBindingSet(shader);
    }

    void Graphics::DestroySwapchain(const Swapchain& swapchain)
    {
        renderDevice->DestroySwapchain(swapchain);
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

    void Graphics::DestroyBindingSet(BindingSet& bindingSet)
    {
        renderDevice->DestroyBindingSet(bindingSet);
    }

    RenderPass Graphics::AcquireNextRenderPass(Swapchain swapchain)
    {
        return renderDevice->AcquireNextRenderPass(swapchain);
    }
}
