#pragma once

#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    struct RenderDevice
    {
        virtual void                Init() = 0;
        virtual Span<GPUAdapter>    GetAdapters() = 0;
        virtual void                CreateDevice(GPUAdapter adapter) = 0;
        virtual Swapchain           CreateSwapchain(const SwapchainCreation& swapchainCreation) = 0;

        virtual void                DestroySwapchain(const Swapchain& swapchain) = 0;

        virtual RenderCommands&     GraphicsBeginFrame() = 0;
        virtual RenderPass          AcquireNextRenderPass(Swapchain swapchain) = 0;
        virtual void                GraphicsEndFrame(Swapchain swapchain) = 0;


        virtual ~RenderDevice() {};
    };
}