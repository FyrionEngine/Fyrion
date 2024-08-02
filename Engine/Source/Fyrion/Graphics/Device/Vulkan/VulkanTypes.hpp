#pragma once

#include "volk.h"
#include "vk_mem_alloc.h"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/FixedArray.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"

namespace Fyrion
{
    class VulkanDevice;

    static const u32 MaxBindlessResources = 16536;

    struct VulkanSwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR  capabilities{};
        Array<VkSurfaceFormatKHR> formats{};
        Array<VkPresentModeKHR>   presentModes{};
    };

    struct VulkanAdapter
    {
        VkPhysicalDevice physicalDevice{};
        u32              score{};
    };

    struct VulkanRenderPass
    {
        VkRenderPass        renderPass;
        VkFramebuffer       framebuffer;
        VkExtent2D          extent{};
        bool                hasDepth;
        Array<VkClearValue> clearValues{};
        Array<VkFormat>     formats{};
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
        BufferCreation    bufferCreation{};
        VkBuffer          buffer{};
        VmaAllocation     allocation{};
        VmaAllocationInfo allocInfo{};
    };

    struct VulkanTextureView
    {
        Texture     texture{};
        VkImageView imageView{};
    };

    struct VulkanTexture
    {
        TextureCreation creation{};
        VkImage         image{};
        VmaAllocation   allocation{};
        TextureView     textureView{};
        VkDescriptorSet imguiDescriptorSet{};
        String          name{};
        u64             id{};
    };

    struct VulkanSampler
    {
        VkSampler sampler{};
    };

    struct VulkanPipelineState
    {
        GraphicsPipelineCreation graphicsPipelineCreation{};
        ComputePipelineCreation  computePipelineCreation{};
        Array<VkFormat>          attachments{};
        VkPipelineBindPoint      bindingPoint{};
        VkPipeline               pipeline{};
        VkPipelineLayout         layout{};
        VkPipelineCache          cache{};
    };
}
