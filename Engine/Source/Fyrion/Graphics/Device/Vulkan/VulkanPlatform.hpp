#pragma once

namespace Fyrion::Platform
{
    void                SetVulkanLoader(PFN_vkGetInstanceProcAddr procAddr);
    Span<const char*>   GetRequiredInstanceExtensions();
    bool                GetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, u32 queueFamily);
    VkResult            CreateWindowSurface(Window window, VkInstance instance, VkSurfaceKHR* surface);
}