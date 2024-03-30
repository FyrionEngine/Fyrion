#pragma once

#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    struct RenderDevice
    {
        virtual Span<GPUAdapter>    GetAdapters() = 0;
        virtual void                CreateDevice(GPUAdapter adapter) = 0;
        virtual Swapchain           CreateSwapchain(const SwapchainCreation& swapchainCreation) = 0;

        virtual void                DestroySwapchain(const Swapchain& swapchain) = 0;

        virtual RenderCommands&     BeginFrame() = 0;
        virtual RenderPass          AcquireNextRenderPass(Swapchain swapchain) = 0;
        virtual void                EndFrame(Swapchain swapchain) = 0;


        virtual ~RenderDevice() {};
    };
}