#pragma once

#include "volk.h"
#include "vk_mem_alloc.h"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/FixedArray.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"

namespace Fyrion
{
    struct VulkanSwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR  capabilities{};
        Array<VkSurfaceFormatKHR> formats{};
        Array<VkPresentModeKHR>   presentModes{};
    };

    struct VulkanAdapter
    {
        VkPhysicalDevice physicalDevice{};
        u32 score{};
    };

    struct VulkanRenderPass
    {
        VkRenderPass        renderPass;
        VkFramebuffer       framebuffer;
        VkExtent2D          extent{};
        bool                hasDepth;
        Array<VkClearValue> clearValues{};
    };

    struct VulkanSwapchain
    {
        Window                  window{};
        bool                    vsync{};
        VkSurfaceKHR            surfaceKHR{};
        VkSwapchainKHR          swapchainKHR{};
        VkExtent2D              extent{};
        VkFormat                format{};
        Array<VkImage>          images{};
        Array<VkImageView>      imageViews{};
        Array<VulkanRenderPass> renderPasses{};
        u32                     imageIndex{};

        FixedArray<VkSemaphore, FY_FRAMES_IN_FLIGHT> imageAvailableSemaphores{};
    };

    struct VulkanBuffer
    {
        VkBuffer          buffer{};
        VmaAllocation     allocation{};
        VmaAllocationInfo allocInfo{};
    };
}