#pragma once

#include "Fyrion/Graphics/Device/RenderDevice.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "VulkanCommands.hpp"

namespace Fyrion
{

    struct VulkanDevice : RenderDevice
    {
        SharedPtr<VulkanCommands> commands;

        void                Init() override;
        Span<GPUAdapter>    GetAdapters() override;
        void                CreateDevice(GPUAdapter adapter) override;
        Swapchain           CreateSwapchain(const SwapchainCreation& swapchainCreation) override;
        void                DestroySwapchain(const Swapchain& swapchain) override;
        RenderCommands&     GraphicsBeginFrame() override;
        RenderPass          AcquireNextRenderPass(Swapchain swapchain) override;
        void                GraphicsEndFrame(Swapchain swapchain) override;
    };


    SharedPtr<RenderDevice> CreateVulkanDevice();

}