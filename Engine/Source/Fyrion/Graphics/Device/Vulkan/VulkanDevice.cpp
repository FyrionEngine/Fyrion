#include "VulkanDevice.hpp"

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"
#include "VulkanPlatform.hpp"
#include "VulkanUtils.hpp"

namespace Fyrion
{


    VulkanDevice::~VulkanDevice()
    {
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

#ifdef SK_MACOS
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
            adapters[i] = GPUAdapter{allocator.Alloc<VulkanAdapter>(devices[i], Vulkan::GetPhysicalDeviceScore(devices[i]))};
        }

        Sort(adapters.begin(), adapters.end(), [](GPUAdapter left, GPUAdapter right)
        {
            return static_cast<VulkanAdapter*>(left.handler)->score > static_cast<VulkanAdapter*>(right.handler)->score;
        });
    }

    Span<GPUAdapter> VulkanDevice::GetAdapters()
    {
        return adapters;
    }

    void VulkanDevice::CreateDevice(GPUAdapter adapter)
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
        deviceFeatures.bindlessSupported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;


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

    Swapchain VulkanDevice::CreateSwapchain(const SwapchainCreation& swapchainCreation)
    {
        return Swapchain();
    }

    void VulkanDevice::DestroySwapchain(const Swapchain& swapchain)
    {

    }

    RenderPass VulkanDevice::AcquireNextRenderPass(Swapchain swapchain)
    {
        return RenderPass();
    }

    RenderCommands& VulkanDevice::BeginFrame()
    {
        return *defaultCommands[currentFrame];
    }

    void VulkanDevice::EndFrame(Swapchain swapchain)
    {

    }


    SharedPtr<RenderDevice> CreateVulkanDevice()
    {
        return MakeShared<VulkanDevice>();
    }
}
