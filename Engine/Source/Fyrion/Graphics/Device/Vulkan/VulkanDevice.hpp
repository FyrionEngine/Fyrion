#pragma once

#include "Fyrion/Graphics/Device/RenderDevice.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "VulkanCommands.hpp"
#include "Fyrion/Core/Logger.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"
#include "Fyrion/Core/FixedArray.hpp"
#include "VulkanTypes.hpp"

namespace Fyrion
{

    struct VulkanDevice : RenderDevice
    {
        Logger&                     logger = Logger::GetLogger("Fyrion::Vulkan");
        Allocator&                  allocator = MemoryGlobals::GetDefaultAllocator();
        VkInstance                  instance{};

        VkPhysicalDevice            physicalDevice{};
        DeviceFeatures              deviceFeatures{};
        VkPhysicalDeviceFeatures    vulkanDeviceFeatures{};
        VkPhysicalDeviceProperties  vulkanDeviceProperties{};
        VkDevice                    device{};
        VmaAllocator                vmaAllocator{};
        VkDescriptorPool            descriptorPool{};
        bool                        maintenance4Available = false;
        Array<GPUAdapter>           adapters{};

        //raytrace
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR    rayTracingPipelineProperties{};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};

        //Validation layers
        const char*                     validationLayers[1] = {"VK_LAYER_KHRONOS_validation"};
        bool                            enableValidationLayers    = false;
        bool                            validationLayersAvailable = false;
        VkDebugUtilsMessengerEXT        debugUtilsMessengerExt{};

        u32                             graphicsFamily{U32_MAX};
        u32                             presentFamily{U32_MAX};
        Array<VkQueueFamilyProperties>  queueFamilies;

        VkQueue                         graphicsQueue{};
        VkQueue                         presentQueue{};

        FixedArray<VkFence, FY_FRAMES_IN_FLIGHT>                    inFlightFences{};
        FixedArray<VkSemaphore, FY_FRAMES_IN_FLIGHT>                renderFinishedSemaphores{};
        FixedArray<SharedPtr<VulkanCommands>, FY_FRAMES_IN_FLIGHT>  defaultCommands{};

        u32 currentFrame = 0;

        VulkanDevice();
        ~VulkanDevice() override;

        Span<GPUAdapter>    GetAdapters() override;
        void                CreateDevice(GPUAdapter adapter) override;
        Swapchain           CreateSwapchain(const SwapchainCreation& swapchainCreation) override;
        void                DestroySwapchain(const Swapchain& swapchain) override;
        RenderCommands&     BeginFrame() override;
        RenderPass          AcquireNextRenderPass(Swapchain swapchain) override;
        void                EndFrame(Swapchain swapchain) override;


        bool                CreateSwapchain(VulkanSwapchain* vulkanSwapchain);
        void                DestroySwapchain(VulkanSwapchain* vulkanSwapchain);

        void ImGuiInit(Swapchain renderSwapchain) override;
        void ImGuiNewFrame() override;
        void ImGuiRender(RenderCommands& renderCommands) override;
    };


    SharedPtr<RenderDevice> CreateVulkanDevice();

}