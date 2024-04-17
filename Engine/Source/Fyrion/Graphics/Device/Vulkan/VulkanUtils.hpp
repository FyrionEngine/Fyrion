#pragma once

#include "VulkanTypes.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion::Vulkan
{
    VkBool32 VKAPI_PTR DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackDataExt, void* userData);

    bool                          QueryLayerProperties(const Span<const char*>& requiredLayers);
    bool                          QueryInstanceExtensions(const Span<const char*>& requiredExtensions);
    u32                           GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice);
    bool                          QueryDeviceExtensions(const Span<VkExtensionProperties>& extensions, const StringView& checkForExtension);
    VulkanSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkSurfaceFormatKHR            ChooseSwapSurfaceFormat(const VulkanSwapChainSupportDetails& supportDetails, VkSurfaceFormatKHR desiredFormat);
    VkPresentModeKHR              ChooseSwapPresentMode(const VulkanSwapChainSupportDetails& supportDetails, VkPresentModeKHR desiredPresentMode);
    VkExtent2D                    ChooseSwapExtent(const VulkanSwapChainSupportDetails& supportDetails, Extent extent);
    VkBufferUsageFlags            CastBufferUsage(BufferUsage bufferUsage);
    VkFormat                      CastFormat(const ImageFormat& textureFormat);
    VkImageUsageFlags             CastTextureUsage(TextureUsage textureUsage);

    inline bool QueryInstanceExtension(const char* extension)
    {
        return QueryInstanceExtensions(Span<const char*>(&extension, &extension + 1));
    }
}
