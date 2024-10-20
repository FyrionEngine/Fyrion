#include "VulkanDevice.hpp"

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"
#include "VulkanBindingSet.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanUtils.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/ImGui/Lib/imgui_impl_vulkan.h"

namespace Fyrion
{
    VulkanDevice::~VulkanDevice()
    {
        ImGui_ImplVulkan_Shutdown();

        DestroySampler(defaultSampler);

        for (size_t i = 0; i < FY_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        if (bindlessDescriptorPool)
        {
            vkDestroyDescriptorPool(device, bindlessDescriptorPool, nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroyCommandPool(device, temporaryCmd->commandPool, nullptr);

        for (int i = 0; i < FY_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroyCommandPool(device, defaultCommands[i]->commandPool, nullptr);
        }

        vmaDestroyAllocator(vmaAllocator);
        vkDestroyDevice(device, nullptr);
        if (validationLayersAvailable)
        {
            vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessengerExt, nullptr);
        }
        vkDestroyInstance(instance, nullptr);
    }

    VulkanDevice::VulkanDevice()
    {
        if (volkInitialize() != VK_SUCCESS)
        {
            logger.FatalError("vulkan cannot be initialized");
        }

#ifdef FY_DEBUG
       enableValidationLayers = true;
#endif

        Platform::SetVulkanLoader(vkGetInstanceProcAddr);

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Fyrion";
        applicationInfo.applicationVersion = 0;
        applicationInfo.pEngineName = "Fyrion";
        applicationInfo.engineVersion = 0;
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &applicationInfo;

        validationLayersAvailable = enableValidationLayers && Vulkan::QueryLayerProperties(Span<const char*>{validationLayers, 1});

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

        if (validationLayersAvailable)
        {
            debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsMessengerCreateInfo.pfnUserCallback = &Vulkan::DebugCallback;
            debugUtilsMessengerCreateInfo.pUserData = &logger;

            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = validationLayers;
            createInfo.pNext = &debugUtilsMessengerCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        Span<const char*>  platformReqExtensions = Platform::GetRequiredInstanceExtensions();
        Array<const char*> requiredExtensions{};
        requiredExtensions.Insert(requiredExtensions.begin(), platformReqExtensions.begin(), platformReqExtensions.end());

        if (validationLayersAvailable)
        {
            requiredExtensions.EmplaceBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            debugUtilsExtensionPresent = true;
        }

        if (!Vulkan::QueryInstanceExtensions(requiredExtensions))
        {
            logger.Error("Required extensions not found");
            FY_ASSERT(false, "Required extensions not found");
        }

#ifdef FY_MACOS
        if (Vulkan::QueryInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
		{
			requiredExtensions.EmplaceBack(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
#endif

        createInfo.enabledExtensionCount = requiredExtensions.Size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.Data();

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        {
            logger.Error("Error on create vkCreateInstance");
            FY_ASSERT(false, "Error on create vkCreateInstance");
        }

        FY_ASSERT(instance, "instance cannot be created");

        volkLoadInstance(instance);

        if (validationLayersAvailable)
        {
            vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugUtilsMessengerExt);
        }

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        Array<VkPhysicalDevice> devices(deviceCount);
        adapters.Resize(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.Data());

        for (int i = 0; i < deviceCount; ++i)
        {
            adapters[i] = Adapter{allocator.Alloc<VulkanAdapter>(devices[i], Vulkan::GetPhysicalDeviceScore(devices[i]))};
        }

        Sort(adapters.begin(), adapters.end(), [](Adapter left, Adapter right)
        {
            return static_cast<VulkanAdapter*>(left.handler)->score > static_cast<VulkanAdapter*>(right.handler)->score;
        });
    }

    Span<Adapter> VulkanDevice::GetAdapters()
    {
        return adapters;
    }

    void VulkanDevice::CreateDevice(Adapter adapter)
    {
        VulkanAdapter* vulkanAdapter = static_cast<VulkanAdapter*>(adapter.handler != nullptr ? adapter.handler : adapters[0].handler);
        physicalDevice = vulkanAdapter->physicalDevice;

        vkGetPhysicalDeviceFeatures(physicalDevice, &vulkanDeviceFeatures);
        vkGetPhysicalDeviceProperties(physicalDevice, &vulkanDeviceProperties);

        deviceFeatures.multiDrawIndirectSupported = vulkanDeviceFeatures.multiDrawIndirect;

        {
            VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr};
            VkPhysicalDeviceFeatures2                  deviceFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures};
            vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
            deviceFeatures.bindlessSupported =
                indexingFeatures.descriptorBindingPartiallyBound &&
                indexingFeatures.runtimeDescriptorArray &&
                indexingFeatures.descriptorBindingSampledImageUpdateAfterBind &&
                indexingFeatures.descriptorBindingStorageImageUpdateAfterBind;
        }


        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        Array<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.Data());

        maintenance4Available = Vulkan::QueryDeviceExtensions(availableExtensions, VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
        deviceFeatures.raytraceSupported = Vulkan::QueryDeviceExtensions(availableExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        deviceAddressAvailable = Vulkan::QueryDeviceExtensions(availableExtensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

        if (deviceFeatures.raytraceSupported)
        {
            rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

            VkPhysicalDeviceProperties2 deviceProperties2{};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &rayTracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

            accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

            VkPhysicalDeviceFeatures2 features2{};
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features2.pNext = &accelerationStructureProperties;
            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
        }

        //get queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        queueFamilies.Resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.Data());

        {
            i32 i = 0;
            for (const auto& queueFamily : queueFamilies)
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && graphicsFamily == U32_MAX)
                {
                    graphicsFamily = i;
                }

                if (Platform::GetPhysicalDevicePresentationSupport(instance, physicalDevice, i))
                {
                    presentFamily = i;
                }

                if (graphicsFamily != U32_MAX && presentFamily != U32_MAX)
                {
                    break;
                }
                i++;
            }
        }

        FY_ASSERT(graphicsFamily != U32_MAX, "Graphics queue not found");
        FY_ASSERT(presentFamily != U32_MAX, "Present queue not found");

        float queuePriority = 1.0f;

        Array<VkDeviceQueueCreateInfo> queueCreateInfos{};
        if (graphicsFamily != presentFamily)
        {
            queueCreateInfos.Resize(2);

            queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[0].queueFamilyIndex = graphicsFamily;
            queueCreateInfos[0].queueCount = 1;
            queueCreateInfos[0].pQueuePriorities = &queuePriority;

            queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[1].queueFamilyIndex = presentFamily;
            queueCreateInfos[1].queueCount = 1;
            queueCreateInfos[1].pQueuePriorities = &queuePriority;
        }
        else
        {
            queueCreateInfos.Resize(1);
            queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[0].queueFamilyIndex = graphicsFamily;
            queueCreateInfos[0].queueCount = 1;
            queueCreateInfos[0].pQueuePriorities = &queuePriority;
        }



        VkDeviceCreateInfo createInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        createInfo.pQueueCreateInfos = queueCreateInfos.Data();
        createInfo.queueCreateInfoCount = queueCreateInfos.Size();

        Array<const char*> deviceExtensions{};
        deviceExtensions.EmplaceBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (maintenance4Available)
        {
            deviceExtensions.EmplaceBack(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
        }

        if (deviceFeatures.raytraceSupported)
        {
            deviceExtensions.EmplaceBack(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            deviceExtensions.EmplaceBack(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            deviceExtensions.EmplaceBack(VK_KHR_RAY_QUERY_EXTENSION_NAME);
            deviceExtensions.EmplaceBack(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensions.EmplaceBack(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
            deviceExtensions.EmplaceBack(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
            deviceExtensions.EmplaceBack(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        }

        if (deviceAddressAvailable)
        {
            deviceExtensions.EmplaceBack(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        }


#ifdef FY_APPLE
        deviceExtensions.EmplaceBack("VK_KHR_portability_subset");
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.Size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.Data();

        if (validationLayersAvailable)
        {
            auto validationLayerSize = sizeof(validationLayers) / sizeof(const char*);
            createInfo.enabledLayerCount = validationLayerSize;
            createInfo.ppEnabledLayerNames = validationLayers;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }


        //--------------  begin feature chain
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

        if (vulkanDeviceFeatures.samplerAnisotropy)
        {
            deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
        }

        if (deviceFeatures.multiDrawIndirectSupported)
        {
            deviceFeatures2.features.multiDrawIndirect = VK_TRUE;
        }

        if (vulkanDeviceFeatures.shaderInt64)
        {
            deviceFeatures2.features.shaderInt64 = VK_TRUE;
        }

        if (vulkanDeviceFeatures.fillModeNonSolid)
        {
            deviceFeatures2.features.fillModeNonSolid = true;
        }

        VkPhysicalDeviceMaintenance4FeaturesKHR vkPhysicalDeviceMaintenance4FeaturesKhr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR};
        vkPhysicalDeviceMaintenance4FeaturesKhr.maintenance4 = true;

        if (maintenance4Available)
        {
            vkPhysicalDeviceMaintenance4FeaturesKhr.pNext = deviceFeatures2.pNext;
            deviceFeatures2.pNext = &vkPhysicalDeviceMaintenance4FeaturesKhr;
        }

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr};

        if (deviceFeatures.bindlessSupported)
        {
            indexingFeatures.descriptorBindingPartiallyBound = true;
            indexingFeatures.runtimeDescriptorArray = true;
            indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
            indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = true;

            indexingFeatures.pNext = deviceFeatures2.pNext;
            deviceFeatures2.pNext = &indexingFeatures;
        }


        VkPhysicalDeviceRayQueryFeaturesKHR deviceRayQueryFeaturesKhr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
        deviceRayQueryFeaturesKhr.rayQuery = true;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR deviceAccelerationStructureFeaturesKhr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
        deviceAccelerationStructureFeaturesKhr.pNext = &deviceRayQueryFeaturesKhr;
        deviceAccelerationStructureFeaturesKhr.accelerationStructure = true;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR deviceRayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
        deviceRayTracingPipelineFeatures.pNext = &deviceAccelerationStructureFeaturesKhr;
        deviceRayTracingPipelineFeatures.rayTracingPipeline = true;

        if (deviceFeatures.raytraceSupported)
        {
            deviceRayTracingPipelineFeatures.pNext = deviceFeatures2.pNext;
            deviceFeatures2.pNext = &deviceRayTracingPipelineFeatures;
        }


        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
        bufferDeviceAddressFeatures.bufferDeviceAddress = true;

        if (deviceAddressAvailable)
        {
            bufferDeviceAddressFeatures.pNext = deviceFeatures2.pNext;
            deviceFeatures2.pNext = &bufferDeviceAddressFeatures;
        }
        //--------------  end feature chain


        //features12.pNext
        createInfo.pNext = &deviceFeatures2;

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            logger.Error("Failed to create device");
        }

        VmaVulkanFunctions vmaVulkanFunctions{};
        vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;

        if (deviceAddressAvailable)
        {
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }

        allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
        vmaCreateAllocator(&allocatorInfo, &vmaAllocator);

        temporaryCmd = MakeShared<VulkanCommands>(*this);

        for (int j = 0; j < FY_FRAMES_IN_FLIGHT; ++j)
        {
            defaultCommands[j] = MakeShared<VulkanCommands>(*this);
        }

        //default descriptor pool
        {
            VkDescriptorPoolSize sizes[5] = {
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 500},
                {VK_DESCRIPTOR_TYPE_SAMPLER, 500},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 500},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 500},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 500}
            };

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = 5;
            poolInfo.pPoolSizes = sizes;
            poolInfo.maxSets = 500;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
        }


        if (deviceFeatures.bindlessSupported)
        {
            VkDescriptorPoolSize sizes[2] = {
                {VK_DESCRIPTOR_TYPE_SAMPLER, MaxBindlessResources},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxBindlessResources},
            };

            VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            poolInfo.poolSizeCount = 2;
            poolInfo.pPoolSizes = sizes;
            poolInfo.maxSets = MaxBindlessResources * 2;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
            vkCreateDescriptorPool(device, &poolInfo, nullptr, &bindlessDescriptorPool);
        }

        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);


        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < FY_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                logger.Error("Failed to create frame objects");
                FY_ASSERT(false, "error");
            }
        }

        defaultSampler = CreateSampler(SamplerCreation{});

        logger.Info("Vulkan API {}.{}.{} Device: {} ",
                    VK_VERSION_MAJOR(vulkanDeviceProperties.apiVersion),
                    VK_VERSION_MINOR(vulkanDeviceProperties.apiVersion),
                    VK_VERSION_PATCH(vulkanDeviceProperties.apiVersion),
                    vulkanDeviceProperties.deviceName);
    }

    bool VulkanDevice::CreateSwapchain(VulkanSwapchain* swapchain)
    {
        if (Platform::CreateWindowSurface(swapchain->window, instance, &swapchain->surfaceKHR) != VK_SUCCESS)
        {
            logger.Error("Error on create CreateWindowSurface");
            return false;
        }

        VulkanSwapChainSupportDetails details = Vulkan::QuerySwapChainSupport(physicalDevice, swapchain->surfaceKHR);
        VkSurfaceFormatKHR            format = Vulkan::ChooseSwapSurfaceFormat(details, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
        VkPresentModeKHR              presentMode = Vulkan::ChooseSwapPresentMode(details, swapchain->vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR);
        Extent                        extent = Platform::GetWindowExtent(swapchain->window);

        swapchain->extent = Vulkan::ChooseSwapExtent(details, {extent.width, extent.height});

        u32 imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
        {
            imageCount = details.capabilities.maxImageCount;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, presentFamily, swapchain->surfaceKHR, &presentSupport);
        if (!presentSupport)
        {
            logger.Error("PhysicalDeviceSurfaceSupportKHR not supported");
            return false;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = swapchain->surfaceKHR;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = swapchain->extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        u32 queueFamilyIndices[] = {graphicsFamily, presentFamily};
        if (graphicsFamily != presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain->swapchainKHR);
        vkGetSwapchainImagesKHR(device, swapchain->swapchainKHR, &imageCount, nullptr);

        swapchain->format = format.format;
        swapchain->images.Resize(imageCount);
        swapchain->imageViews.Resize(imageCount);
        swapchain->renderPasses.Resize(imageCount);

        vkGetSwapchainImagesKHR(device, swapchain->swapchainKHR, &imageCount, swapchain->images.Data());

        for (size_t i = 0; i < imageCount; i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapchain->images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = format.format;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchain->imageViews[i]);
        }

        for (int i = 0; i < imageCount; ++i)
        {
            VulkanRenderPass& vulkanRenderPass = swapchain->renderPasses[i];
            vulkanRenderPass.extent = swapchain->extent;
            vulkanRenderPass.clearValues.Resize(1);
            vulkanRenderPass.formats.EmplaceBack(format.format);

            VkAttachmentDescription attachmentDescription{};
            VkAttachmentReference   colorAttachmentReference{};

            attachmentDescription.format = format.format;
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentReference.attachment = 0;

            VkSubpassDescription subPass = {};
            subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subPass.colorAttachmentCount = 1;
            subPass.pColorAttachments = &colorAttachmentReference;

            VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &attachmentDescription;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subPass;
            renderPassInfo.dependencyCount = 0;
            vkCreateRenderPass(device, &renderPassInfo, nullptr, &vulkanRenderPass.renderPass);

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = vulkanRenderPass.renderPass;
            framebufferCreateInfo.width = vulkanRenderPass.extent.width;
            framebufferCreateInfo.height = vulkanRenderPass.extent.height;
            framebufferCreateInfo.layers = 1u;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = &swapchain->imageViews[i];
            vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &vulkanRenderPass.framebuffer);
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (int i = 0; i < FY_FRAMES_IN_FLIGHT; ++i)
        {
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &swapchain->imageAvailableSemaphores[i]);
        }

        return true;
    }

    void VulkanDevice::DestroySwapchain(VulkanSwapchain* swapchain)
    {
        if (swapchain == nullptr) return;

        vkDeviceWaitIdle(device);

        for (int i = 0; i < swapchain->renderPasses.Size(); ++i)
        {
            vkDestroyRenderPass(device, swapchain->renderPasses[i].renderPass, nullptr);
            vkDestroyFramebuffer(device, swapchain->renderPasses[i].framebuffer, nullptr);
        }

        for (int i = 0; i < swapchain->imageViews.Size(); ++i)
        {
            vkDestroyImageView(device, swapchain->imageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain->swapchainKHR, nullptr);
        vkDestroySurfaceKHR(instance, swapchain->surfaceKHR, nullptr);

        for (int i = 0; i < FY_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(device, swapchain->imageAvailableSemaphores[i], nullptr);
        }
    }

    Swapchain VulkanDevice::CreateSwapchain(const SwapchainCreation& swapchainCreation)
    {
        VulkanSwapchain* vulkanSwapchain = allocator.Alloc<VulkanSwapchain>(VulkanSwapchain{
            .window = swapchainCreation.window,
            .vsync = swapchainCreation.vsync
        });
        return CreateSwapchain(vulkanSwapchain) ? Swapchain{vulkanSwapchain} : Swapchain{};
    }

    RenderPass VulkanDevice::CreateRenderPass(const RenderPassCreation& renderPassCreation)
    {
        VulkanRenderPass* vulkanRenderPass = allocator.Alloc<VulkanRenderPass>();

        Array<VkAttachmentDescription> attachmentDescriptions{};
        Array<VkAttachmentReference>   colorAttachmentReference{};
        Array<VkImageView>             imageViews{};
        VkAttachmentReference          depthReference{};

        bool     hasDepth = false;
        Extent3D framebufferSize{};

		vulkanRenderPass->clearValues.Resize(renderPassCreation.attachments.Size());

		for (int i = 0; i < renderPassCreation.attachments.Size(); ++i)
		{
			const AttachmentCreation& attachment = renderPassCreation.attachments[i];

			VkFormat format;

            if (VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(attachment.texture.handler))
            {
                imageViews.EmplaceBack(static_cast<VulkanTextureView*>(vulkanTexture->textureView.handler)->imageView);
                format = Vulkan::CastFormat(vulkanTexture->creation.format);
                framebufferSize = vulkanTexture->creation.extent;
            }
            else if (VulkanTextureView* vulkanTextureView = static_cast<VulkanTextureView*>(attachment.textureView.handler))
            {
                imageViews.EmplaceBack(vulkanTextureView->imageView);
                VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(vulkanTextureView->texture.handler);
                format = Vulkan::CastFormat(vulkanTexture->creation.format);
                framebufferSize = vulkanTexture->creation.extent;
            }
            else
            {
                FY_ASSERT(false, "texture or texture view must be provieded");
            }

			bool isDepthFormat = Vulkan::IsDepthFormat(format);

			VkAttachmentDescription attachmentDescription{};
			attachmentDescription.format = format;
			attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

			attachmentDescription.loadOp = Vulkan::CastLoadOp(attachment.loadOp);
			attachmentDescription.storeOp = Vulkan::CastStoreOp(attachment.storeOp);

			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			attachmentDescription.initialLayout = Vulkan::CastLayout(attachment.initialLayout);

			if (!isDepthFormat)
			{
				attachmentDescription.finalLayout = Vulkan::CastLayout(attachment.finalLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				VkAttachmentReference reference{};
				reference.attachment = i;
				reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachmentReference.EmplaceBack(reference);
			    vulkanRenderPass->formats.EmplaceBack(format);
			}
			else
			{
				attachmentDescription.finalLayout = Vulkan::CastLayout(attachment.finalLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				depthReference.attachment = i;
				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				hasDepth = true;
			}
			attachmentDescriptions.EmplaceBack(attachmentDescription);
		}

		vulkanRenderPass->extent = {framebufferSize.width, framebufferSize.height};

		VkSubpassDescription subPass = {};
		subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPass.colorAttachmentCount = colorAttachmentReference.Size();
		subPass.pColorAttachments = colorAttachmentReference.Data();
		if (hasDepth)
		{
			subPass.pDepthStencilAttachment = &depthReference;
			vulkanRenderPass->hasDepth = true;
		}

		VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		renderPassInfo.attachmentCount = attachmentDescriptions.Size();
		renderPassInfo.pAttachments = attachmentDescriptions.Data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subPass;
		renderPassInfo.dependencyCount = 0;
		vkCreateRenderPass(device, &renderPassInfo, nullptr, &vulkanRenderPass->renderPass);

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = vulkanRenderPass->renderPass;
		framebufferCreateInfo.width = framebufferSize.width;
		framebufferCreateInfo.height = framebufferSize.height;
		framebufferCreateInfo.layers = std::max(framebufferSize.depth, 1u);
		framebufferCreateInfo.attachmentCount = imageViews.Size();
		framebufferCreateInfo.pAttachments = imageViews.Data();

		vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &vulkanRenderPass->framebuffer);

        return {vulkanRenderPass};
    }

    Buffer VulkanDevice::CreateBuffer(const BufferCreation& bufferCreation)
    {
        FY_ASSERT(bufferCreation.size > 0, "size cannot be 0");

        VulkanBuffer* vulkanBuffer = allocator.Alloc<VulkanBuffer>(bufferCreation);

        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = bufferCreation.size;
        bufferInfo.usage = Vulkan::CastBufferUsage(bufferCreation.usage);

        VmaAllocationCreateInfo vmaAllocInfo = {};

        switch (bufferCreation.allocation)
        {
            case BufferAllocation::GPUOnly:
            {
                vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                break;
            }

            case BufferAllocation::TransferToGPU:
            {
                vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                vmaAllocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                break;
            }

            case BufferAllocation::TransferToCPU:
            {
                vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                vmaAllocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                break;
            }
        }
        if (vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaAllocInfo, &vulkanBuffer->buffer, &vulkanBuffer->allocation, &vulkanBuffer->allocInfo) != VK_SUCCESS)
        {
            FY_ASSERT(false, "error on create buffer");
        }
        FY_ASSERT(vulkanBuffer->buffer, "buffer not created");
        return {vulkanBuffer};
    }

    Texture VulkanDevice::CreateTexture(const TextureCreation& textureCreation)
    {
        static u64 idCounter = 0;
        VulkanTexture* vulkanTexture = allocator.Alloc<VulkanTexture>(textureCreation);
        vulkanTexture->id = idCounter++;

        VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = Vulkan::CastFormat(textureCreation.format);
        imageCreateInfo.extent = {textureCreation.extent.width, textureCreation.extent.height, Math::Max(textureCreation.extent.depth, 1u)};
        imageCreateInfo.mipLevels = textureCreation.mipLevels;
        imageCreateInfo.arrayLayers = textureCreation.arrayLayers;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

        TextureUsage usage = textureCreation.usage;
        if (usage == TextureUsage::None)
        {
            usage |= TextureUsage::TransferDst;
            usage |= TextureUsage::ShaderResource;
        }
        imageCreateInfo.usage = Vulkan::CastTextureUsage(usage);

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        TextureViewCreation textureViewCreation{
            .viewType = textureCreation.defaultView != ViewType::Undefined ? textureCreation.defaultView : ViewType::Type2D,
            .levelCount = textureCreation.mipLevels,
            .layerCount = imageCreateInfo.arrayLayers
        };

        if (imageCreateInfo.arrayLayers % 6 == 0)
        {
            textureViewCreation.viewType = ViewType::TypeCube;
            imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }
        else if (imageCreateInfo.arrayLayers > 1)
        {
            textureViewCreation.viewType = ViewType::Type2DArray;
        }

        vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocInfo, &vulkanTexture->image, &vulkanTexture->allocation, nullptr);

        textureViewCreation.texture = {vulkanTexture};
        vulkanTexture->textureView = CreateTextureView(textureViewCreation);

        if(!textureCreation.name.Empty())
        {
            vulkanTexture->name = textureCreation.name;
            Vulkan::SetObjectName(*this, VK_OBJECT_TYPE_IMAGE, (u64)vulkanTexture->image, vulkanTexture->name);
        }

        return {vulkanTexture};
    }

    TextureView VulkanDevice::CreateTextureView(const TextureViewCreation& textureViewCreation)
    {
        VulkanTextureView* vulkanTextureView = allocator.Alloc<VulkanTextureView>();
        vulkanTextureView->texture = textureViewCreation.texture;

        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(textureViewCreation.texture.handler);

        VkImageViewCreateInfo viewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewCreateInfo.viewType = Vulkan::CastViewType(textureViewCreation.viewType);
        viewCreateInfo.image = vulkanTexture->image;
        viewCreateInfo.format = Vulkan::CastFormat(vulkanTexture->creation.format);
        viewCreateInfo.subresourceRange.baseMipLevel = textureViewCreation.baseMipLevel;
        viewCreateInfo.subresourceRange.levelCount = textureViewCreation.levelCount;
        viewCreateInfo.subresourceRange.baseArrayLayer = textureViewCreation.baseArrayLayer;
        viewCreateInfo.subresourceRange.layerCount = textureViewCreation.layerCount;

        if (Vulkan::IsDepthFormat(viewCreateInfo.format))
        {
            viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else
        {
            viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        vkCreateImageView(device, &viewCreateInfo, nullptr, &vulkanTextureView->imageView);

        return {vulkanTextureView};
    }

    Sampler VulkanDevice::CreateSampler(const SamplerCreation& samplerCreation)
    {
        VulkanSampler* vulkanSampler = allocator.Alloc<VulkanSampler>();
        VkSamplerCreateInfo vkSamplerInfo{};
        vkSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vkSamplerInfo.magFilter = Vulkan::CastFilter(samplerCreation.filter);
        vkSamplerInfo.minFilter = Vulkan::CastFilter(samplerCreation.filter);
        vkSamplerInfo.addressModeU = Vulkan::CastTextureAddressMode(samplerCreation.addressMode);
        vkSamplerInfo.addressModeV = vkSamplerInfo.addressModeU;
        vkSamplerInfo.addressModeW = vkSamplerInfo.addressModeU;
        vkSamplerInfo.anisotropyEnable = samplerCreation.anisotropyEnable && vulkanDeviceFeatures.samplerAnisotropy;
        vkSamplerInfo.maxAnisotropy = vulkanDeviceProperties.limits.maxSamplerAnisotropy;
        vkSamplerInfo.borderColor = Vulkan::CasterBorderColor(samplerCreation.borderColor);
        vkSamplerInfo.unnormalizedCoordinates = VK_FALSE;
        vkSamplerInfo.compareEnable = VK_FALSE;
        vkSamplerInfo.compareOp =  Vulkan::CastCompareOp(samplerCreation.compareOperator);
        vkSamplerInfo.mipmapMode = Vulkan::CastSamplerMipmapMode(samplerCreation.samplerMipmapMode);
        vkSamplerInfo.mipLodBias = samplerCreation.mipLodBias;
        vkSamplerInfo.minLod = samplerCreation.minLod;
        vkSamplerInfo.maxLod = samplerCreation.maxLod;
        vkCreateSampler(device, &vkSamplerInfo, nullptr, &vulkanSampler->sampler);
        return {vulkanSampler};
    }

    PipelineState VulkanDevice::CreateGraphicsPipelineState(const GraphicsPipelineCreation& creation)
    {
        ShaderAsset* shader = creation.shader;
        FY_ASSERT(shader, "shader is null");
        bool invalidPass = creation.attachments.Empty() && !creation.renderPass && !creation.pipelineState && creation.depthFormat == Format::Undefined;
        FY_ASSERT(!invalidPass, "creation needs attachments or renderpass or pipelineState");


        Span<ShaderStageInfo> stages = shader->stages;
        ShaderInfo            shaderInfo = shader->shaderInfo;


        VulkanPipelineState* vulkanPipelineState;
        if (!creation.pipelineState)
        {
            vulkanPipelineState = allocator.Alloc<VulkanPipelineState>();
            vulkanPipelineState->graphicsPipelineCreation = creation;

            shader->AddPipelineDependency({vulkanPipelineState});
        }
        else
        {
            vulkanPipelineState = static_cast<VulkanPipelineState*>(creation.pipelineState.handler);

            vkDeviceWaitIdle(device);
            vkDestroyPipelineLayout(device, vulkanPipelineState->layout, nullptr);
            vkDestroyPipeline(device, vulkanPipelineState->pipeline, nullptr);
        }

        Array<VkPushConstantRange>                 pushConstants{};
        Array<VkPipelineColorBlendAttachmentState> attachments{};
        Array<VkVertexInputAttributeDescription>   attributeDescriptions{};

        Array<VkShaderModule> shaderModules{};
        shaderModules.Resize(stages.Size());
        Array<VkPipelineShaderStageCreateInfo> shaderStages{};
        shaderStages.Resize(stages.Size());

        for (u32 i = 0; i < stages.Size(); ++i)
        {
            const ShaderStageInfo& shaderStageAsset = stages[i];

            Span<u8> data = Span<u8>{
                shader->bytes.begin() + shaderStageAsset.offset,
                shader->bytes.begin() + shaderStageAsset.offset + shaderStageAsset.size
            };

            VkShaderModuleCreateInfo createInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            createInfo.codeSize = data.Size();
            createInfo.pCode = reinterpret_cast<const u32*>(data.Data());
            vkCreateShaderModule(device, &createInfo, nullptr, &shaderModules[i]);

            shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[i].module = shaderModules[i];
            shaderStages[i].pName = shaderStageAsset.entryPoint.CStr();
            shaderStages[i].stage = static_cast<VkShaderStageFlagBits>(Vulkan::CastStage(shaderStageAsset.stage));
        }

        for (ShaderPushConstant pushConstant : shaderInfo.pushConstants)
        {
            pushConstants.EmplaceBack(VkPushConstantRange{
                .stageFlags = Vulkan::CastStage(pushConstant.stage),
                .offset = pushConstant.offset,
                .size = pushConstant.size
            });
        }

        u32 stride = shaderInfo.stride;
        if (creation.stride > 0)
        {
            stride = creation.stride;
        }

        if (!creation.inputs.Empty())
        {
            for (const auto& input : creation.inputs)
            {
                attributeDescriptions.EmplaceBack(VkVertexInputAttributeDescription{
                    .location = input.location,
                    .binding = 0,
                    .format = Vulkan::CastFormat(input.format),
                    .offset = input.offset
                });
            }
        }
        else
        {
            for (const auto& input : shaderInfo.inputVariables)
            {
                attributeDescriptions.EmplaceBack(VkVertexInputAttributeDescription{
                    .location = input.location,
                    .binding = 0,
                    .format = Vulkan::CastFormat(input.format),
                    .offset = input.offset
                });
            }
        }


        for (const auto& output : shaderInfo.outputVariables)
        {
            VkPipelineColorBlendAttachmentState attachmentState{};
            attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            if (vulkanPipelineState->graphicsPipelineCreation.blendEnabled)
            {
                attachmentState.blendEnable = VK_TRUE;
                attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
                attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
            }
            else
            {
                attachmentState.blendEnable = VK_FALSE;
            }
            attachments.EmplaceBack(attachmentState);
        }

        Vulkan::CreatePipelineLayout(device, shaderInfo.descriptors, shaderInfo.pushConstants, &vulkanPipelineState->layout);

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = stride;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

        if (bindingDescription.stride > 0)
        {
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        }
        else
        {
            vertexInputInfo.vertexBindingDescriptionCount = 0;
        }

        if (!attributeDescriptions.Empty())
        {
            vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.Size();
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.Data();
        } else
        {
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
        }


        VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inputAssembly.topology = Vulkan::CastPrimitiveTopology(vulkanPipelineState->graphicsPipelineCreation.primitiveTopology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = Vulkan::CastPolygonMode(vulkanPipelineState->graphicsPipelineCreation.polygonMode);
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = Vulkan::CastCull(vulkanPipelineState->graphicsPipelineCreation.cullMode);
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlending{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = attachments.Size();
        colorBlending.pAttachments = attachments.Data();
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        FixedArray<VkDynamicState, 2>    dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicState.pDynamicStates = dynamicStateEnables.Data();
        dynamicState.dynamicStateCount = dynamicStateEnables.Size();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.Size();
        pipelineInfo.pStages = shaderStages.Data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = vulkanPipelineState->layout;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (creation.renderPass && vulkanPipelineState->attachments.Empty())
        {
            VulkanRenderPass* renderPass = static_cast<VulkanRenderPass*>(vulkanPipelineState->graphicsPipelineCreation.renderPass.handler);

            for (VkFormat format : renderPass->formats)
            {
                if (!Vulkan::IsDepthFormat(format))
                {
                    vulkanPipelineState->attachments.EmplaceBack(format);
                }
            }

            if (renderPass->hasDepth)
            {
                vulkanPipelineState->graphicsPipelineCreation.depthFormat = Format::Depth;
            }
        }

        if (!creation.attachments.Empty() && vulkanPipelineState->attachments.Empty())
        {
            for(Format format: creation.attachments)
            {
                vulkanPipelineState->attachments.EmplaceBack(Vulkan::CastFormat(format));
            }
        }

        Array<VkAttachmentDescription> attachmentDescriptions{};
        Array<VkAttachmentReference>   colorAttachmentReference{};
        VkAttachmentReference          depthReference{};

        for (VkFormat attachmentFormat : vulkanPipelineState->attachments)
        {
            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.format = attachmentFormat;
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentDescriptions.EmplaceBack(attachmentDescription);

            VkAttachmentReference reference{};
            reference.attachment = colorAttachmentReference.Size();
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentReference.EmplaceBack(reference);
        }

        if (vulkanPipelineState->graphicsPipelineCreation.depthFormat != Format::Undefined)
        {
            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.format = VK_FORMAT_D32_SFLOAT;
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDescriptions.EmplaceBack(attachmentDescription);

            depthReference.attachment = colorAttachmentReference.Size();
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subPass = {};
        subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPass.colorAttachmentCount = colorAttachmentReference.Size();
        subPass.pColorAttachments = colorAttachmentReference.Data();
        if (vulkanPipelineState->graphicsPipelineCreation.depthFormat != Format::Undefined)
        {
            subPass.pDepthStencilAttachment = &depthReference;
        }

        VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        renderPassInfo.attachmentCount = attachmentDescriptions.Size();
        renderPassInfo.pAttachments = attachmentDescriptions.Data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subPass;
        renderPassInfo.dependencyCount = 0;
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &pipelineInfo.renderPass);

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        if (vulkanPipelineState->graphicsPipelineCreation.depthFormat != Format::Undefined)
        {
            auto depthTest = vulkanPipelineState->graphicsPipelineCreation.compareOperator != CompareOp::Always;
            depthStencilStateCreateInfo.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
            depthStencilStateCreateInfo.depthWriteEnable = vulkanPipelineState->graphicsPipelineCreation.depthWrite ? VK_TRUE : VK_FALSE;
            depthStencilStateCreateInfo.depthCompareOp = Vulkan::CastCompareOp(vulkanPipelineState->graphicsPipelineCreation.compareOperator);
            depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
            depthStencilStateCreateInfo.minDepthBounds = vulkanPipelineState->graphicsPipelineCreation.minDepthBounds;
            depthStencilStateCreateInfo.maxDepthBounds = vulkanPipelineState->graphicsPipelineCreation.maxDepthBounds;
            depthStencilStateCreateInfo.stencilTestEnable = vulkanPipelineState->graphicsPipelineCreation.stencilTest ? VK_TRUE : VK_FALSE;
            depthStencilStateCreateInfo.pNext = nullptr;

            pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        }

        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vulkanPipelineState->pipeline);

        for (const auto shaderModule : shaderModules)
        {
            vkDestroyShaderModule(device, shaderModule, nullptr);
        }

        vkDestroyRenderPass(device, pipelineInfo.renderPass, nullptr);

        return {vulkanPipelineState};
    }

    PipelineState VulkanDevice::CreateComputePipelineState(const ComputePipelineCreation& creation)
    {
        ShaderAsset* shader = creation.shader;
        FY_ASSERT(shader, "shader is null");



        ShaderStageInfo& stage = shader->stages[0];
        ShaderInfo       shaderInfo = shader->shaderInfo;

        VkShaderModule shaderModule{};
        VulkanPipelineState* vulkanPipelineState;
        if (!creation.pipelineState)
        {
            vulkanPipelineState = allocator.Alloc<VulkanPipelineState>();
            vulkanPipelineState->computePipelineCreation = creation;
            shader->AddPipelineDependency({vulkanPipelineState});
        }
        else
        {
            vulkanPipelineState = static_cast<VulkanPipelineState*>(creation.pipelineState.handler);

            vkDeviceWaitIdle(device);
            vkDestroyPipelineLayout(device, vulkanPipelineState->layout, nullptr);
            vkDestroyPipeline(device, vulkanPipelineState->pipeline, nullptr);
        }

        vulkanPipelineState->bindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        VkShaderModuleCreateInfo createInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        createInfo.codeSize = shader->bytes.Size();
        createInfo.pCode = reinterpret_cast<const u32*>(shader->bytes.Data());
        vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

        Vulkan::CreatePipelineLayout(device, shaderInfo.descriptors, shaderInfo.pushConstants, &vulkanPipelineState->layout);

        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.pName = stage.entryPoint.CStr();
        shaderStage.module = shaderModule;

        VkComputePipelineCreateInfo computePipelineCreateInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        computePipelineCreateInfo.layout = vulkanPipelineState->layout;
        computePipelineCreateInfo.stage = shaderStage;

        vkCreateComputePipelines(device, 0, 1, &computePipelineCreateInfo, nullptr, &vulkanPipelineState->pipeline);
        vkDestroyShaderModule(device, shaderModule, nullptr);

        return {vulkanPipelineState};
    }

    BindingSet* VulkanDevice::CreateBindingSet(ShaderAsset* shaderAsset)
    {
        return allocator.Alloc<VulkanBindingSet>(shaderAsset, *this);
    }

    BindingSet* VulkanDevice::CreateBindingSet(Span<DescriptorLayout> descriptorLayouts)
    {
        return allocator.Alloc<VulkanBindingSet>(descriptorLayouts, *this);
    }

    void VulkanDevice::DestroySwapchain(const Swapchain& swapchain)
    {
        VulkanSwapchain* vulkanSwapchain = static_cast<VulkanSwapchain*>(swapchain.handler);
        DestroySwapchain(vulkanSwapchain);
        allocator.DestroyAndFree(vulkanSwapchain);
    }

    void VulkanDevice::DestroyRenderPass(const RenderPass& renderPass)
    {
        VulkanRenderPass* vulkanRenderPass = static_cast<VulkanRenderPass*>(renderPass.handler);
        vkDestroyFramebuffer(device, vulkanRenderPass->framebuffer, nullptr);
        vkDestroyRenderPass(device, vulkanRenderPass->renderPass, nullptr);
        allocator.DestroyAndFree(vulkanRenderPass);
    }

    void VulkanDevice::DestroyBuffer(const Buffer& buffer)
    {
        VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer.handler);
        if (vulkanBuffer->buffer && vulkanBuffer->allocation)
        {
            vmaDestroyBuffer(vmaAllocator, vulkanBuffer->buffer, vulkanBuffer->allocation);
        }
        allocator.DestroyAndFree(vulkanBuffer);
    }

    void VulkanDevice::DestroyTexture(const Texture& texture)
    {
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.handler);

        if (vulkanTexture->imguiDescriptorSet)
        {
            vkFreeDescriptorSets(device, descriptorPool, 1, &vulkanTexture->imguiDescriptorSet);
        }

        if (vulkanTexture->textureView.handler)
        {
            DestroyTextureView(vulkanTexture->textureView);
        }

        if (vulkanTexture->allocation)
        {
            vmaDestroyImage(vmaAllocator, vulkanTexture->image, vulkanTexture->allocation);
            vulkanTexture->allocation = nullptr;
        }
        allocator.DestroyAndFree(vulkanTexture);
    }

    void VulkanDevice::DestroyTextureView(const TextureView& textureView)
    {
        VulkanTextureView* vulkanTextureView = static_cast<VulkanTextureView*>(textureView.handler);
        vkDestroyImageView(device, vulkanTextureView->imageView, nullptr);
        allocator.DestroyAndFree(vulkanTextureView);
    }

    void VulkanDevice::DestroySampler(const Sampler& sampler)
    {
        VulkanSampler* vulkanSampler = static_cast<VulkanSampler*>(sampler.handler);
        vkDestroySampler(device, vulkanSampler->sampler, nullptr);
        allocator.DestroyAndFree(vulkanSampler);
    }

    void VulkanDevice::DestroyGraphicsPipelineState(const PipelineState& pipelineState)
    {
        VulkanPipelineState* vulkanPipelineState = static_cast<VulkanPipelineState*>(pipelineState.handler);
        if (vulkanPipelineState->pipeline)
        {
            vkDestroyPipeline(device, vulkanPipelineState->pipeline, nullptr);
        }
        if (vulkanPipelineState->layout)
        {
            vkDestroyPipelineLayout(device, vulkanPipelineState->layout, nullptr);
        }
        allocator.DestroyAndFree(vulkanPipelineState);
    }

    void VulkanDevice::DestroyComputePipelineState(const PipelineState& pipelineState)
    {
        if (VulkanPipelineState* vulkanPipelineState = static_cast<VulkanPipelineState*>(pipelineState.handler))
        {
            if (vulkanPipelineState->pipeline)
            {
                vkDestroyPipeline(device, vulkanPipelineState->pipeline, nullptr);
            }
            if (vulkanPipelineState->layout)
            {
                vkDestroyPipelineLayout(device, vulkanPipelineState->layout, nullptr);
            }
            allocator.DestroyAndFree(vulkanPipelineState);
        }
    }

    void VulkanDevice::DestroyBindingSet(BindingSet* bindingSet)
    {
        allocator.DestroyAndFree(static_cast<VulkanBindingSet*>(bindingSet));
    }

    RenderPass VulkanDevice::AcquireNextRenderPass(Swapchain swapchain)
    {
        VulkanSwapchain* vulkanSwapchain = static_cast<VulkanSwapchain*>(swapchain.handler);
        Extent           extent = Platform::GetWindowExtent(vulkanSwapchain->window);
        if (extent.width != vulkanSwapchain->extent.width || extent.height != vulkanSwapchain->extent.height)
        {
            while (extent.width == 0 || extent.height == 0)
            {
                extent = Platform::GetWindowExtent(vulkanSwapchain->window);
                Platform::WaitEvents();
            }
            vkDeviceWaitIdle(device);
            DestroySwapchain(vulkanSwapchain);
            CreateSwapchain(vulkanSwapchain);
        }

        VkResult result = vkAcquireNextImageKHR(device,
                                                vulkanSwapchain->swapchainKHR,
                                                UINT64_MAX,
                                                vulkanSwapchain->imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE,
                                                &vulkanSwapchain->imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            FY_ASSERT(false, "failed to acquire swap chain image!");
        }

        return {&vulkanSwapchain->renderPasses[vulkanSwapchain->imageIndex]};
    }

    RenderCommands& VulkanDevice::BeginFrame()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        return *defaultCommands[currentFrame];
    }

    void VulkanDevice::EndFrame(Swapchain swapchain)
    {
        VulkanSwapchain* vulkanSwapchain = static_cast<VulkanSwapchain*>(swapchain.handler);

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &vulkanSwapchain->imageAvailableSemaphores[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &defaultCommands[currentFrame]->commandBuffer;
        submitInfo.pWaitDstStageMask = waitStages;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            FY_ASSERT(false, "failed to execute vkQueueSubmit");
        }

        VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vulkanSwapchain->swapchainKHR;
        presentInfo.pImageIndices = &vulkanSwapchain->imageIndex;

        if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS)
        {
            FY_ASSERT(false, "failed to execute vkQueuePresentKHR");
        }

        currentFrame = (currentFrame + 1) % FY_FRAMES_IN_FLIGHT;
    }

    void VulkanDevice::WaitQueue()
    {
        vkQueueWaitIdle(graphicsQueue);
        vkDeviceWaitIdle(device);
    }

    GPUQueue VulkanDevice::GetMainQueue()
    {
        return {graphicsQueue};
    }

    RenderCommands& VulkanDevice::GetTempCmd()
    {
        return *temporaryCmd;
    }

    void VulkanDevice::UpdateBufferData(const BufferDataInfo& bufferDataInfo)
    {
        FY_ASSERT(bufferDataInfo.data, "data cannot be null");
        FY_ASSERT(bufferDataInfo.size > 0, "size should be higher then zero");

        VulkanBuffer& vulkanBuffer = *static_cast<VulkanBuffer*>(bufferDataInfo.buffer.handler);
        if (vulkanBuffer.bufferCreation.allocation != BufferAllocation::GPUOnly)
        {
            if (bufferDataInfo.data)
            {
                memcpy((i8*)vulkanBuffer.allocInfo.pMappedData + bufferDataInfo.offset, bufferDataInfo.data, bufferDataInfo.size);
            }
        }
        else
        {
            VulkanBuffer stagingBuffer = {};

            VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufferInfo.size = bufferDataInfo.size;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo vmaAllocInfo = {};
            vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaAllocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocInfo);

            memcpy(stagingBuffer.allocInfo.pMappedData, bufferDataInfo.data, bufferDataInfo.size);

            temporaryCmd->Begin();

            BufferCopyInfo copy{};
            copy.dstOffset = 0;
            copy.srcOffset = bufferDataInfo.offset;
            copy.size = bufferDataInfo.size;

            temporaryCmd->CopyBuffer({&stagingBuffer}, bufferDataInfo.buffer, &copy);
            temporaryCmd->SubmitAndWait({graphicsQueue});

            vmaDestroyBuffer(vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);
        }
    }

    VoidPtr VulkanDevice::GetBufferMappedMemory(const Buffer& buffer)
    {
        VulkanBuffer& vulkanBuffer = *static_cast<VulkanBuffer*>(buffer.handler);
        return vulkanBuffer.allocInfo.pMappedData;
    }

    TextureCreation VulkanDevice::GetTextureCreationInfo(Texture texture)
    {
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.handler);
        return vulkanTexture->creation;
    }

    static void CheckVkResult(VkResult err)
    {
        if (err == 0)
        {
            return;
        }
        //logger.Error("VkResult = {}", (i32) err);
    }

    void VulkanDevice::ImGuiInit(Swapchain renderSwapchain)
    {
        VulkanSwapchain* vulkanSwapchain = static_cast<VulkanSwapchain*>(renderSwapchain.handler);

        ImGui_ImplVulkan_LoadFunctions([](const char* functionName, void* userData)
        {
            return vkGetInstanceProcAddr(static_cast<VulkanDevice*>(userData)->instance, functionName);
        }, this);

        ImGui_ImplVulkan_InitInfo Info = {};
        Info.Instance = instance;
        Info.PhysicalDevice = physicalDevice;
        Info.Device = device;
        Info.QueueFamily = graphicsFamily;
        Info.Queue = graphicsQueue;
        Info.PipelineCache = VK_NULL_HANDLE;
        Info.DescriptorPool = descriptorPool;
        Info.UseDynamicRendering = false;
        Info.Subpass = 0;
        Info.MinImageCount = 2;
        Info.ImageCount = vulkanSwapchain->images.Size();
        Info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        Info.Allocator = nullptr;
        Info.RenderPass = vulkanSwapchain->renderPasses[0].renderPass;
        Info.CheckVkResultFn = CheckVkResult;

        ImGui_ImplVulkan_Init(&Info);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void VulkanDevice::ImGuiNewFrame()
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void VulkanDevice::ImGuiRender(RenderCommands& renderCommands)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VulkanCommands&>(renderCommands).commandBuffer);
    }

    VoidPtr VulkanDevice::GetImGuiTexture(const Texture& texture)
    {
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.handler);
        if (vulkanTexture->imguiDescriptorSet == nullptr)
        {
            VulkanTextureView* textureView = static_cast<VulkanTextureView*>(vulkanTexture->textureView.handler);
            VulkanSampler* vulkanSampler = static_cast<VulkanSampler*>(defaultSampler.handler);
            vulkanTexture->imguiDescriptorSet = ImGui_ImplVulkan_AddTexture(vulkanSampler->sampler, textureView->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        return vulkanTexture->imguiDescriptorSet;
    }


    SharedPtr<RenderDevice> CreateVulkanDevice()
    {
        return MakeShared<VulkanDevice>();
    }
}
