
#include "Graphics.hpp"
#include "Fyrion/Graphics/Device/RenderDevice.hpp"
#include "Fyrion/Core/SharedPtr.hpp"

namespace Fyrion
{
    void GraphicsInit();
    void GraphicsShutdown();
    void GraphicsCreateDevice(GPUAdapter adapter);

    SharedPtr<RenderDevice> CreateVulkanDevice();

    namespace
    {
        SharedPtr<RenderDevice> renderDevice = {};
    }

    void GraphicsInit()
    {
        renderDevice = CreateVulkanDevice();
        renderDevice->Init();
    }

    void GraphicsShutdown()
    {
        renderDevice = nullptr;
    }

    void GraphicsCreateDevice(GPUAdapter adapter)
    {
        renderDevice->CreateDevice(adapter);
    }

    RenderCommands& GraphicsBeginFrame()
    {
        return renderDevice->GraphicsBeginFrame();
    }

    void GraphicsEndFrame(Swapchain swapchain)
    {
        renderDevice->GraphicsEndFrame(swapchain);
    }

    Span<GPUAdapter> Graphics::GetAdapters()
    {
        return renderDevice->GetAdapters();
    }

    Swapchain Graphics::CreateSwapchain(const SwapchainCreation& swapchainCreation)
    {
        return renderDevice->CreateSwapchain(swapchainCreation);
    }

    void Graphics::DestroySwapchain(const Swapchain& swapchain)
    {
        renderDevice->DestroySwapchain(swapchain);
    }

    RenderPass Graphics::AcquireNextRenderPass(Swapchain swapchain)
    {
        return renderDevice->AcquireNextRenderPass(swapchain);
    }
}