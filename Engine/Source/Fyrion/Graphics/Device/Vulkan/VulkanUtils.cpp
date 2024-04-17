#include <algorithm>
#include "VulkanUtils.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Logger.hpp"

namespace Fyrion::Vulkan
{

	VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackDataExt,
		void* userData)
	{
		Logger& logger = *static_cast<Logger*>(userData);

		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				logger.Trace("{}", callbackDataExt->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				logger.Info("{}", callbackDataExt->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				logger.Warn("{}", callbackDataExt->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				logger.Error("{}", callbackDataExt->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
				break;
		}

		return VK_FALSE;
	}

	bool QueryLayerProperties(const Span<const char*>& requiredLayers)
	{
		u32 extensionCount = 0;
		vkEnumerateInstanceLayerProperties(&extensionCount, nullptr);
		Array<VkLayerProperties> extensions(extensionCount);
		vkEnumerateInstanceLayerProperties(&extensionCount, extensions.Data());

		for (const StringView& reqExtension: requiredLayers)
		{
			bool found = false;
			for (const auto& layer: extensions)
			{
				if (layer.layerName == StringView(reqExtension))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				return false;
			}
		}

		return true;
	}


	bool QueryDeviceExtensions(const Span<VkExtensionProperties>& extensions, const StringView& checkForExtension)
	{
		for(const VkExtensionProperties& extension: extensions)
		{
			if (StringView(extension.extensionName) == checkForExtension)
			{
				return true;
			}
		}
		return false;
	}

	bool QueryInstanceExtensions(const Span<const char*>& requiredExtensions)
	{
		u32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		Array<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.Data());

		for (const auto reqExtension: requiredExtensions)
		{
			bool found = false;
			for (const auto& extension: extensions)
			{
				if (StringView(extension.extensionName) == StringView(reqExtension))
				{
					found = true;
				}
			}

			if (!found)
			{
				return false;
			}
		}
		return true;
	}

	u32 GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice)
	{
		u32 score = 0;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		score = deviceProperties.limits.maxImageDimension2D;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score *= 3;
		}
		return score;
	}

    VulkanSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
        VulkanSwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		u32 formatCount{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.Resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.Data());
		}
		u32 presentModeCount{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.Resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.Data());
		}
		return details;
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const VulkanSwapChainSupportDetails& supportDetails, VkSurfaceFormatKHR desiredFormat)
	{
		for (const auto& availableFormat: supportDetails.formats)
		{
			if (availableFormat.format == desiredFormat.format && availableFormat.colorSpace == desiredFormat.colorSpace)
			{
				return availableFormat;
			}
		}
		return supportDetails.formats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const VulkanSwapChainSupportDetails& supportDetails, VkPresentModeKHR desiredPresentMode)
	{
		for (const auto& availablePresentMode: supportDetails.presentModes)
		{
			if (availablePresentMode == desiredPresentMode)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(const VulkanSwapChainSupportDetails& supportDetails, Extent extent)
	{
		if (supportDetails.capabilities.currentExtent.width != U32_MAX)
		{
			return supportDetails.capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = {extent.width, extent.height};
			actualExtent.width = std::clamp(actualExtent.width, supportDetails.capabilities.minImageExtent.width, supportDetails.capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, supportDetails.capabilities.minImageExtent.height, supportDetails.capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	VkBufferUsageFlags CastBufferUsage(BufferUsage bufferUsage)
	{
		VkBufferUsageFlags flags{};
		if (bufferUsage && BufferUsage::VertexBuffer)
		{
			flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (bufferUsage && BufferUsage::IndexBuffer)
		{
			flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}

		if (bufferUsage && BufferUsage::UniformBuffer)
		{
			flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (bufferUsage && BufferUsage::StorageBuffer)
		{
			flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (bufferUsage && BufferUsage::IndirectBuffer)
		{
			flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		}
		if (bufferUsage && BufferUsage::AccelerationStructureBuild)
		{
			flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		}

		if (bufferUsage && BufferUsage::AccelerationStructureStorage)
		{
			flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
		}
		return flags;
	}

	VkFormat CastFormat(const ImageFormat& textureFormat)
	{
		switch (textureFormat)
		{
		case ImageFormat::R: return VK_FORMAT_R8_UNORM;
		case ImageFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
		case ImageFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ImageFormat::BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::Depth: return VK_FORMAT_D32_SFLOAT;           //TODO should check if format is available
		case ImageFormat::R16F: return VK_FORMAT_R16_SFLOAT;
		case ImageFormat::R32F: return VK_FORMAT_R32_SFLOAT;
		case ImageFormat::RG: return VK_FORMAT_R8G8_UNORM;
		case ImageFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
		case ImageFormat::RGB: return VK_FORMAT_R8G8B8_UNORM;
		case ImageFormat::RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;
		case ImageFormat::RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;
		case ImageFormat::Undefined:
			break;
		default:
			FY_ASSERT(false, "[VulkanDevice] VkFormat not found");
			return VK_FORMAT_UNDEFINED;
		}
		return {};
	}

	VkImageUsageFlags CastTextureUsage(TextureUsage textureUsage)
	{
		VkImageUsageFlags usage{};
		if (textureUsage && TextureUsage::ShaderResource)
		{
			usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}

		if (textureUsage && TextureUsage::DepthStencil)
		{
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		if (textureUsage && TextureUsage::RenderPass)
		{
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		if (textureUsage && TextureUsage::Storage)
		{
			usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		if (textureUsage && TextureUsage::TransferDst)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		if (textureUsage && TextureUsage::TransferSrc)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		return usage;
	}
}

