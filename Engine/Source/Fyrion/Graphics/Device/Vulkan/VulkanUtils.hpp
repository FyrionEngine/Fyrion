#pragma once

#include "VulkanTypes.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion::Vulkan
{
    VkBool32 VKAPI_PTR DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackDataExt,
                                     void*                                  userData);

    bool                          QueryLayerProperties(const Span<const char*>& requiredLayers);
    bool                          QueryInstanceExtensions(const Span<const char*>& requiredExtensions);
    u32                           GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice);
    bool                          QueryDeviceExtensions(const Span<VkExtensionProperties>& extensions, const StringView& checkForExtension);
    VulkanSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkSurfaceFormatKHR            ChooseSwapSurfaceFormat(const VulkanSwapChainSupportDetails& supportDetails, VkSurfaceFormatKHR desiredFormat);
    VkPresentModeKHR              ChooseSwapPresentMode(const VulkanSwapChainSupportDetails& supportDetails, VkPresentModeKHR desiredPresentMode);
    VkExtent2D                    ChooseSwapExtent(const VulkanSwapChainSupportDetails& supportDetails, Extent extent);
    VkBufferUsageFlags            CastBufferUsage(BufferUsage bufferUsage);
    VkFormat                      CastFormat(const Format& textureFormat);
    VkImageUsageFlags             CastTextureUsage(TextureUsage textureUsage);
    void                          CreateDescriptorSetLayout(VkDevice vkDevice, const DescriptorLayout& descriptor, VkDescriptorSetLayout* descriptorSetLayout, bool* hasRuntimeArray = nullptr);
    void                          CreatePipelineLayout(VkDevice vkDevice, Array<DescriptorLayout>& descriptors, Array<ShaderPushConstant>& pushConstants, VkPipelineLayout* vkPipelineLayout);
    VkShaderStageFlags            CastStage(const ShaderStage& shaderStage);
    VkPolygonMode                 CastPolygonMode(const PolygonMode& polygonMode);
    VkCullModeFlags               CastCull(const CullMode& cullMode);
    VkCompareOp                   CastCompareOp(const CompareOp& compareOp);
    VkDescriptorType              CastDescriptorType(const DescriptorType& descriptorType);
    VkPrimitiveTopology           CastPrimitiveTopology(const PrimitiveTopology& primitiveTopology);
    VkImageLayout                 CastLayout(const ResourceLayout& resourceLayout, VkImageLayout defaultUndefined = VK_IMAGE_LAYOUT_UNDEFINED);
    VkAttachmentLoadOp            CastLoadOp(LoadOp loadOp);
    VkAttachmentStoreOp           CastStoreOp(StoreOp storeOp);
    VkImageViewType               CastViewType(const ViewType& viewType);
    VkFilter                      CastFilter(const SamplerFilter& samplerFilter);
    VkBorderColor                 CasterBorderColor(BorderColor borderColor);
    VkSamplerAddressMode          CastTextureAddressMode(const TextureAddressMode& mode);
    VkSamplerMipmapMode           CastSamplerMipmapMode(SamplerMipmapMode samplerMipmapMode);
    void                          SetObjectName(VulkanDevice& device, VkObjectType type, u64 handle, StringView name);

    inline bool QueryInstanceExtension(const char* extension)
    {
        return QueryInstanceExtensions(Span<const char*>(&extension, &extension + 1));
    }

    constexpr bool IsDepthFormat(VkFormat format)
    {
        return (format == VK_FORMAT_D32_SFLOAT_S8_UINT
            || format == VK_FORMAT_D32_SFLOAT
            || format == VK_FORMAT_D24_UNORM_S8_UINT
            || format == VK_FORMAT_D16_UNORM_S8_UINT
            || format == VK_FORMAT_D16_UNORM);
    }
}
