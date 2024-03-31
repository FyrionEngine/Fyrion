#pragma once

#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion::Graphics
{
    FY_API Span<GPUAdapter> GetAdapters();
    FY_API Swapchain        CreateSwapchain(const SwapchainCreation& swapchainCreation);
    FY_API void             DestroySwapchain(const Swapchain& swapchain);
    FY_API RenderPass       AcquireNextRenderPass(Swapchain swapchain);
}