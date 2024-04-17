#include "VulkanDevice.hpp"

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"
#include "VulkanBindingSet.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanUtils.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/ImGui/Lib/imgui_impl_vulkan.h"

namespace Fyrion
{


    VulkanDevice::~VulkanDevice()
    {
        ImGui_ImplVulkan_Shutdown();

        for (size_t i = 0; i < FY_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

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
        volkInitialize();

#ifdef FY_DEBUG
        enableValidationLayers = true;
#endif

        Platform::SetVulkanLoader(vkGetInstanceProcAddr);

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName   = "Fyrion";
        applicationInfo.applicationVersion = 0;
        applicationInfo.pEngineName        = "Fyrion";
        applicationInfo.engineVersion      = 0;
        applicationInfo.apiVersion         = VK_API_VERSION_1_3;


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
        }

        if (!Vulkan::QueryInstanceExtensions(requiredExtensions))
        {
            logger.Error("Required extensions not found");
            FY_ASSERT(false, "Required extensions not found");
        }

#ifdef FY_MACOS
        if (QueryInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
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

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
        VkPhysicalDeviceFeatures2 deviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures };
        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
        //TODO check other extensions for bindless.
        deviceFeatures.bindlessSupported = false; //indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;


        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        Array<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.Data());

        maintenance4Available = Vulkan::QueryDeviceExtensions(availableExtensions, VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
        deviceFeatures.raytraceSupported = Vulkan::QueryDeviceExtensions(availableExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

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
            for (const auto& queueFamily: queueFamilies)
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
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


        float queuePriority = 1.0f;

        //**raytrace***
        VkPhysicalDeviceRayQueryFeaturesKHR deviceRayQueryFeaturesKhr{};
        deviceRayQueryFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        deviceRayQueryFeaturesKhr.pNext = nullptr;
        deviceRayQueryFeaturesKhr.rayQuery = VK_TRUE;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR deviceAccelerationStructureFeaturesKhr{};

        deviceAccelerationStructureFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        deviceAccelerationStructureFeaturesKhr.pNext = &deviceRayQueryFeaturesKhr;
        deviceAccelerationStructureFeaturesKhr.accelerationStructure = VK_TRUE;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR deviceRayTracingPipelineFeatures{};
        deviceRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        deviceRayTracingPipelineFeatures.pNext = &deviceAccelerationStructureFeaturesKhr;
        deviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
        //endraytrace.

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

        VkPhysicalDeviceFeatures physicalDeviceFeatures{};

        if (vulkanDeviceFeatures.samplerAnisotropy)
        {
            physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
        }

        if (deviceFeatures.multiDrawIndirectSupported)
        {
            physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;
        }

        physicalDeviceFeatures.shaderInt64 = VK_TRUE;
        physicalDeviceFeatures.fillModeNonSolid = true;

        VkPhysicalDeviceVulkan12Features features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        features12.bufferDeviceAddress = VK_TRUE;

        VkPhysicalDeviceMaintenance4FeaturesKHR vkPhysicalDeviceMaintenance4FeaturesKhr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR};
        vkPhysicalDeviceMaintenance4FeaturesKhr.maintenance4 = true;

        if (deviceFeatures.raytraceSupported)
        {
            vkPhysicalDeviceMaintenance4FeaturesKhr.pNext = &deviceRayTracingPipelineFeatures;
        }

        if (maintenance4Available)
        {
            features12.pNext = &vkPhysicalDeviceMaintenance4FeaturesKhr;
        }

        if (deviceFeatures.bindlessSupported)
        {
            //TODO do I need all these features?
            features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            features12.runtimeDescriptorArray = VK_TRUE;
            features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
            features12.descriptorBindingPartiallyBound = VK_TRUE;
            features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
            features12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        }

        VkDeviceCreateInfo createInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        createInfo.pQueueCreateInfos = queueCreateInfos.Data();
        createInfo.queueCreateInfoCount = queueCreateInfos.Size();
        createInfo.pEnabledFeatures = &physicalDeviceFeatures;

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
            deviceExtensions.EmplaceBack(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        }

#if __APPLE_CC__
        deviceExtensions.EmplaceBack("VK_KHR_portability_subset");
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.Size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.Data();

        if (validationLayersAvailable)
        {
            auto validationLayerSize = sizeof(validationLayers) / sizeof(const char*);
            createInfo.enabledLayerCount   = validationLayerSize;
            createInfo.ppEnabledLayerNames = validationLayers;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        createInfo.pNext = &features12;

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
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
        vmaCreateAllocator(&allocatorInfo, &vmaAllocator);

        for (int j = 0; j < FY_FRAMES_IN_FLIGHT; ++j)
        {
            defaultCommands[j] = MakeShared<VulkanCommands>(*this);
        }

        VkDescriptorPoolSize sizes[4] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         500},
            {VK_DESCRIPTOR_TYPE_SAMPLER,                500},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          500},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         500}
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 4;
        poolInfo.pPoolSizes = sizes;
        poolInfo.maxSets = 500;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (deviceFeatures.bindlessSupported)
        {
            poolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
        }

        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
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

        VulkanSwapChainSupportDetails details   = Vulkan::QuerySwapChainSupport(physicalDevice, swapchain->surfaceKHR);
        VkSurfaceFormatKHR      format          = Vulkan::ChooseSwapSurfaceFormat(details, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
        VkPresentModeKHR        presentMode     = Vulkan::ChooseSwapPresentMode(details, swapchain->vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR);
        Extent                  extent          = Platform::GetWindowExtent(swapchain->window);

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

            VkAttachmentDescription attachmentDescription{};
            VkAttachmentReference colorAttachmentReference{};

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
        VulkanRenderPass*              vulkanRenderPass = allocator.Alloc<VulkanRenderPass>();

        Array<VkAttachmentDescription> attachmentDescriptions{};
        Array<VkAttachmentReference>   colorAttachmentReference{};
        Array<VkImageView>             imageViews{};
        VkAttachmentReference          depthReference{};

        vulkanRenderPass->clearValues.Resize(renderPassCreation.attachments.Size());

        for (u32 i = 0; i < renderPassCreation.attachments.Size(); ++i)
        {
            const AttachmentCreation& attachment = renderPassCreation.attachments[i];
            VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(attachment.texture.handler);
            FY_ASSERT(vulkanTexture, "texture not provided");

            //imageViews.EmplaceBack(static_cast<VulkanTextureView*>(vulkanTexture->textureView.handler)->imageView);

            // VkFormat format = Vulkan::CastFormat(vulkanTexture->textureCreation.format);
            // framebufferSize = vulkanTexture->textureCreation.extent;
            // bool isDepthFormat = IsDepthFormat(format);
        }

        VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        renderPassInfo.attachmentCount = attachmentDescriptions.Size();
        renderPassInfo.pAttachments = attachmentDescriptions.Data();
        renderPassInfo.subpassCount = 1;
      //  renderPassInfo.pSubpasses = &subPass;
        renderPassInfo.dependencyCount = 0;
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &vulkanRenderPass->renderPass);

        return {vulkanRenderPass};
    }

    Buffer VulkanDevice::CreateBuffer(const BufferCreation& bufferCreation)
    {
        VulkanBuffer* vulkanBuffer = allocator.Alloc<VulkanBuffer>();

        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = bufferCreation.size;
        bufferInfo.usage = Vulkan::CastBufferUsage(bufferCreation.usage);

        VmaAllocationCreateInfo vmaAllocInfo = {};
        vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

        switch (bufferCreation.memory)
        {
        case BufferMemory::GPUOnly:
            {
                vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                break;
            }
        case BufferMemory::Dynamic: //TODO dynamic buffer should be VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT with a stagging.
        case BufferMemory::TransferToGPU:
            {
                vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                break;
            }
        case BufferMemory::TransferToCPU:
            {
                vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            break;
        }
        vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaAllocInfo, &vulkanBuffer->buffer, &vulkanBuffer->allocation, &vulkanBuffer->allocInfo);
        return {vulkanBuffer};
    }

    Texture VulkanDevice::CreateTexture(const TextureCreation& textureCreation)
    {
        VulkanTexture* vulkanTexture = allocator.Alloc<VulkanTexture>(textureCreation);

        VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = Vulkan::CastFormat(textureCreation.format);
        imageCreateInfo.extent = {textureCreation.extent.width, textureCreation.extent.height, std::max(textureCreation.extent.depth, 1u)};
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

        vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocInfo, &vulkanTexture->image, &vulkanTexture->allocation, nullptr);

        Texture texture = Texture{vulkanTexture};

        TextureViewCreation textureViewCreation{
            .texture = texture,
            .viewType = textureCreation.defaultView != ViewType::Undefined ? textureCreation.defaultView : ViewType::Type2D,
            .levelCount = textureCreation.mipLevels,
            .layerCount = imageCreateInfo.arrayLayers
        };

        if (imageCreateInfo.arrayLayers % 6 == 0)
        {
            textureViewCreation.viewType = ViewType::TypeCube;
            imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        vulkanTexture->textureView = CreateTextureView(textureViewCreation);

        return texture;
    }

    TextureView VulkanDevice::CreateTextureView(const TextureViewCreation& textureViewCreation)
    {
        return {};
    }

    Sampler VulkanDevice::CreateSampler(const SamplerCreation& samplerCreation)
    {
        return {};
    }

    PipelineState VulkanDevice::CreateGraphicsPipelineState(const GraphicsPipelineCreation& graphicsPipelineCreation)
    {
        return {};
    }

    PipelineState VulkanDevice::CreateComputePipelineState(const ComputePipelineCreation& computePipelineCreation)
    {
        return {};
    }

    BindingSet& VulkanDevice::CreateBindingSet(RID shader)
    {
        return *allocator.Alloc<VulkanBindingSet>();
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

    }

    void VulkanDevice::DestroyTextureView(const TextureView& textureView)
    {
    }

    void VulkanDevice::DestroySampler(const Sampler& sampler)
    {
    }

    void VulkanDevice::DestroyGraphicsPipelineState(const PipelineState& pipelineState)
    {
    }

    void VulkanDevice::DestroyComputePipelineState(const PipelineState& pipelineState)
    {
    }

    void VulkanDevice::DestroyBindingSet(BindingSet& bindingSet)
    {
        allocator.DestroyAndFree(static_cast<VulkanBindingSet*>(&bindingSet));
    }

    RenderPass VulkanDevice::AcquireNextRenderPass(Swapchain swapchain)
    {
        VulkanSwapchain* vulkanSwapchain = static_cast<VulkanSwapchain*>(swapchain.handler);
        Extent extent = Platform::GetWindowExtent(vulkanSwapchain->window);
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
        Info.Instance        = instance;
        Info.PhysicalDevice  = physicalDevice;
        Info.Device          = device;
        Info.QueueFamily     = graphicsFamily;
        Info.Queue           = graphicsQueue;
        Info.PipelineCache   = VK_NULL_HANDLE;
        Info.DescriptorPool  = descriptorPool;
        Info.Subpass         = 0;
        Info.MinImageCount   = 2;
        Info.ImageCount      = vulkanSwapchain->images.Size();
        Info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;
        Info.Allocator       = nullptr;
        Info.RenderPass      = vulkanSwapchain->renderPasses[0].renderPass;
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

    SharedPtr<RenderDevice> CreateVulkanDevice()
    {
        return MakeShared<VulkanDevice>();
    }
}
