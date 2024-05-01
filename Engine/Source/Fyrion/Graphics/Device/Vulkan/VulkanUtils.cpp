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

	VkFormat CastFormat(const Format& textureFormat)
	{
		switch (textureFormat)
		{
		case Format::R: return VK_FORMAT_R8_UNORM;
		case Format::RG16F: return VK_FORMAT_R16G16_SFLOAT;
		case Format::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Format::BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::Depth: return VK_FORMAT_D32_SFLOAT;           //TODO should check if format is available
		case Format::R16F: return VK_FORMAT_R16_SFLOAT;
		case Format::R32F: return VK_FORMAT_R32_SFLOAT;
		case Format::RG: return VK_FORMAT_R8G8_UNORM;
		case Format::RG32F: return VK_FORMAT_R32G32_SFLOAT;
		case Format::RGB: return VK_FORMAT_R8G8B8_UNORM;
		case Format::RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;
		case Format::RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;
		case Format::Undefined:
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

	void CreateDescriptorSetLayout(VkDevice vkDevice, const DescriptorLayout& descriptor, VkDescriptorSetLayout* descriptorSetLayout, bool* hasRuntimeArrays)
	{
		Array<VkDescriptorSetLayoutBinding> bindings{};
		bindings.Resize(descriptor.bindings.Size());

		bool hasRuntimeArrayValue{};

		for (int i = 0; i < descriptor.bindings.Size(); ++i)
		{
			bindings[i].binding = descriptor.bindings[i].binding;
			bindings[i].descriptorCount = descriptor.bindings[i].renderType != RenderType::RuntimeArray ? descriptor.bindings[i].count : MaxBindlessResources;
			bindings[i].descriptorType = CastDescriptorType(descriptor.bindings[i].descriptorType);
			bindings[i].stageFlags = CastStage(descriptor.bindings[i].shaderStage);

			if (descriptor.bindings[i].renderType == RenderType::RuntimeArray)
			{
				hasRuntimeArrayValue = true;
			}
		}

		Array<VkDescriptorBindingFlags> bindlessFlags{};

		if (hasRuntimeArrayValue)
		{
			bindlessFlags.Resize(bindings.Size());
			for (int i = 0; i < bindings.Size(); ++i)
			{
				if (descriptor.bindings[i].renderType == RenderType::RuntimeArray)
				{
					bindlessFlags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
				}
			}
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr};
		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

		setLayoutCreateInfo.bindingCount = bindings.Size();
		setLayoutCreateInfo.pBindings = bindings.Data();

		if (hasRuntimeArrayValue)
		{
			extendedInfo.bindingCount = bindlessFlags.Size();
			extendedInfo.pBindingFlags = bindlessFlags.Data();

			setLayoutCreateInfo.pNext = &extendedInfo;
			setLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		}
		vkCreateDescriptorSetLayout(vkDevice, &setLayoutCreateInfo, nullptr, descriptorSetLayout);

		if (hasRuntimeArrayValue)
		{
			*hasRuntimeArrays = hasRuntimeArrayValue;
		}
	}

	void CreatePipelineLayout(VkDevice vkDevice, Array<DescriptorLayout>& descriptors, Array<ShaderPushConstant>& pushConstants, VkPipelineLayout* vkPipelineLayout)
	{
		Array<VkDescriptorSetLayout> descriptorSetLayouts{};
		descriptorSetLayouts.Resize(descriptors.Size());


		Array<VkPushConstantRange> pushConstantRanges{};

		for(const ShaderPushConstant& pushConstant: pushConstants)
		{
			pushConstantRanges.EmplaceBack(VkPushConstantRange{
				.stageFlags = CastStage(pushConstant.stage),
				.offset = pushConstant.offset,
				.size = pushConstant.size
			});
		}

		for (int i = 0; i < descriptors.Size(); ++i)
		{
			CreateDescriptorSetLayout(vkDevice, descriptors[i], &descriptorSetLayouts[i]);
		}

		VkPipelineLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		layoutCreateInfo.setLayoutCount = descriptorSetLayouts.Size();
		layoutCreateInfo.pSetLayouts = descriptorSetLayouts.Data();
		layoutCreateInfo.pPushConstantRanges = pushConstantRanges.Data();
		layoutCreateInfo.pushConstantRangeCount = pushConstantRanges.Size();

		vkCreatePipelineLayout(vkDevice, &layoutCreateInfo, nullptr, vkPipelineLayout);

		for (int i = 0; i < descriptorSetLayouts.Size(); ++i)
		{
			vkDestroyDescriptorSetLayout(vkDevice, descriptorSetLayouts[i], nullptr);
		}
	}

	VkShaderStageFlags CastStage(const ShaderStage& shaderStage)
	{
		VkShaderStageFlags stage{0};

		if (shaderStage && ShaderStage::Vertex)
		{
			stage |= VK_SHADER_STAGE_VERTEX_BIT;
		}

		if (shaderStage && ShaderStage::Pixel)
		{
			stage |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		if (shaderStage && ShaderStage::Compute)
		{
			stage |= VK_SHADER_STAGE_COMPUTE_BIT;
		}
		if (shaderStage && ShaderStage::RayGen)
		{
			stage |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		}
		if (shaderStage && ShaderStage::RayMiss)
		{
			stage |= VK_SHADER_STAGE_MISS_BIT_KHR;
		}
		if (shaderStage && ShaderStage::RayClosestHit)
		{
			stage |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		}
		if (shaderStage && ShaderStage::All)
		{
			stage |= VK_SHADER_STAGE_ALL;
		}

		FY_ASSERT(stage != 0, "Shader stage missing");

		return stage;
	}

	VkPolygonMode CastPolygonMode(const PolygonMode& polygonMode)
	{
		switch (polygonMode)
		{
			case PolygonMode::Fill : return VK_POLYGON_MODE_FILL;
			case PolygonMode::Line : return VK_POLYGON_MODE_LINE;
			case PolygonMode::Point : return VK_POLYGON_MODE_POINT;
		}
		FY_ASSERT(false, "VulkanUtils.cpp:  castPolygonMode not found");
		return VK_POLYGON_MODE_MAX_ENUM;
	}

	VkCullModeFlags CastCull(const CullMode& cullMode)
	{
		switch (cullMode)
		{
			case CullMode::None: return VK_CULL_MODE_NONE;
			case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
			case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
		}
		FY_ASSERT(false, "VulkanUtils.cpp:  castCull not found");
		return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}

	VkCompareOp CastCompareOp(const CompareOp& compareOp)
	{
		switch (compareOp)
		{
			case CompareOp::Never:return VK_COMPARE_OP_NEVER;
			case CompareOp::Less:return VK_COMPARE_OP_LESS;
			case CompareOp::Equal:return VK_COMPARE_OP_EQUAL;
			case CompareOp::LessOrEqual:return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOp::Greater:return VK_COMPARE_OP_GREATER;
			case CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
			case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
		}
		FY_ASSERT(false, "VulkanUtils.hpp:  castCompareOp not found");
		return VK_COMPARE_OP_MAX_ENUM;
	}

	VkDescriptorType CastDescriptorType(const DescriptorType& descriptorType)
	{
		switch (descriptorType)
		{
			case DescriptorType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
			case DescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case DescriptorType::AccelerationStructure: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		}
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	VkPrimitiveTopology CastPrimitiveTopology(const PrimitiveTopology& primitiveTopology)
	{
		switch (primitiveTopology)
		{
			case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			case PrimitiveTopology::LineListWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
			case PrimitiveTopology::LineStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
			case PrimitiveTopology::TriangleListWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
			case PrimitiveTopology::TriangleStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
			case PrimitiveTopology::PatchList: break;
		}
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	VkImageLayout CastLayout(const ResourceLayout& resourceLayout, VkImageLayout defaultUndefined)
	{
		switch (resourceLayout)
		{
			case ResourceLayout::Undefined: return defaultUndefined;
			case ResourceLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
			case ResourceLayout::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case ResourceLayout::DepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case ResourceLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case ResourceLayout::CopyDest: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case ResourceLayout::CopySource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case ResourceLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		FY_ASSERT(false, "VulkanUtils.hpp: castLayout not found");
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	VkAttachmentLoadOp CastLoadOp(LoadOp loadOp)
	{
		switch (loadOp)
		{
			case LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
			case LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		FY_ASSERT(false, "VulkanUtils.hpp: castLoadOp not found");
		return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}

	VkAttachmentStoreOp CastStoreOp(StoreOp storeOp)
	{
		switch (storeOp)
		{
			case StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
			case StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		FY_ASSERT(false, "VulkanUtils.hpp: castStoreOp not found");
		return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}

	VkImageViewType CastViewType(const ViewType& viewType)
	{
		switch(viewType)
		{
			case ViewType::Type1D: return VK_IMAGE_VIEW_TYPE_1D;
			case ViewType::Type2D: return VK_IMAGE_VIEW_TYPE_2D;
			case ViewType::Type3D: return VK_IMAGE_VIEW_TYPE_3D;
			case ViewType::TypeCube: return VK_IMAGE_VIEW_TYPE_CUBE;
			case ViewType::Type1DArray: return VK_IMAGE_VIEW_TYPE_1D;
			case ViewType::Type2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			case ViewType::TypeCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			case ViewType::Undefined:break;
		}
		FY_ASSERT(false, "VulkanUtils.hpp: CastLayout not found");
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}

	VkFilter CastFilter(const SamplerFilter& samplerFilter)
	{
		switch (samplerFilter)
		{
			case SamplerFilter::Nearest: return VK_FILTER_NEAREST;
			case SamplerFilter::Linear: return VK_FILTER_LINEAR;
			case SamplerFilter::CubicImg: return VK_FILTER_CUBIC_IMG;
		}

		FY_ASSERT(false, "VulkanUtils.hpp: CastFilter not filter");
		return VK_FILTER_MAX_ENUM;
	}

	VkBorderColor CasterBorderColor(BorderColor borderColor)
	{
		switch (borderColor)
		{
			case BorderColor::FloatTransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case BorderColor::IntTransparentBlack: return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
			case BorderColor::FloatOpaqueBlack: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case BorderColor::IntOpaqueBlack: return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case BorderColor::FloatOpaqueWhite: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		}

		FY_ASSERT(false, "VulkanUtils.hpp: CasterBorderColor");
		return VK_BORDER_COLOR_MAX_ENUM;
	}

	VkSamplerAddressMode CastTextureAddressMode(const TextureAddressMode& mode)
	{
		switch (mode)
		{
			case TextureAddressMode::Repeat:return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case TextureAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case TextureAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case TextureAddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case TextureAddressMode::MirrorClampToEdge: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}
		FY_ASSERT(false, "VulkanUtils.hpp: CastTextureAddressMode not found");
		return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
	}

	VkSamplerMipmapMode CastSamplerMipmapMode(SamplerMipmapMode samplerMipmapMode)
	{
		switch (samplerMipmapMode)
		{
			case SamplerMipmapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case SamplerMipmapMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}
		FY_ASSERT(false, "VulkanUtils.hpp: CastTextureAddressMode not found");
		return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
	}
}

