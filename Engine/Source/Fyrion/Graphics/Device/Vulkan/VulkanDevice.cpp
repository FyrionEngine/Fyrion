#include "VulkanDevice.hpp"

namespace Fyrion
{

    void VulkanDevice::Init()
    {

    }

    Span<GPUAdapter> VulkanDevice::GetAdapters()
    {
        return Span<GPUAdapter>();
    }

    void VulkanDevice::CreateDevice(GPUAdapter adapter)
    {
        commands = MakeShared<VulkanCommands>(*this);
    }

    Swapchain VulkanDevice::CreateSwapchain(const SwapchainCreation& swapchainCreation)
    {
        return Swapchain();
    }

    void VulkanDevice::DestroySwapchain(const Swapchain& swapchain)
    {

    }

    RenderPass VulkanDevice::AcquireNextRenderPass(Swapchain swapchain)
    {
        return RenderPass();
    }

    RenderCommands& VulkanDevice::GraphicsBeginFrame()
    {
        return *commands;
    }

    void VulkanDevice::GraphicsEndFrame(Swapchain swapchain)
    {

    }

    SharedPtr<RenderDevice> CreateVulkanDevice()
    {
        return MakeShared<VulkanDevice>();
    }
}
