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
    class VulkanDevice final : public RenderDevice
    {
    public:
        Logger&    logger = Logger::GetLogger("Fyrion::Vulkan");
        Allocator& allocator = MemoryGlobals::GetDefaultAllocator();
        VkInstance instance{};

        VkPhysicalDevice           physicalDevice{};
        DeviceFeatures             deviceFeatures{};
        VkPhysicalDeviceFeatures   vulkanDeviceFeatures{};
        VkPhysicalDeviceProperties vulkanDeviceProperties{};
        VkDevice                   device{};
        VmaAllocator               vmaAllocator{};
        VkDescriptorPool           descriptorPool{};
        bool                       maintenance4Available = false;
        Array<Adapter>             adapters{};

        //raytrace
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR    rayTracingPipelineProperties{};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};

        //Validation layers
        const char*              validationLayers[1] = {"VK_LAYER_KHRONOS_validation"};
        bool                     enableValidationLayers = false;
        bool                     validationLayersAvailable = false;
        VkDebugUtilsMessengerEXT debugUtilsMessengerExt{};

        u32                            graphicsFamily{U32_MAX};
        u32                            presentFamily{U32_MAX};
        Array<VkQueueFamilyProperties> queueFamilies;

        VkQueue graphicsQueue{};
        VkQueue presentQueue{};
        SharedPtr<VulkanCommands> temporaryCmd;

        FixedArray<VkFence, FY_FRAMES_IN_FLIGHT>                   inFlightFences{};
        FixedArray<VkSemaphore, FY_FRAMES_IN_FLIGHT>               renderFinishedSemaphores{};
        FixedArray<SharedPtr<VulkanCommands>, FY_FRAMES_IN_FLIGHT> defaultCommands{};

        u32 currentFrame = 0;

        VulkanDevice();
        ~VulkanDevice() override;

        Span<Adapter>   GetAdapters() override;
        void            CreateDevice(Adapter adapter) override;
        Swapchain       CreateSwapchain(const SwapchainCreation& swapchainCreation) override;
        RenderPass      CreateRenderPass(const RenderPassCreation& renderPassCreation) override;
        Buffer          CreateBuffer(const BufferCreation& bufferCreation) override;
        Texture         CreateTexture(const TextureCreation& textureCreation) override;
        TextureView     CreateTextureView(const TextureViewCreation& textureViewCreation) override;
        Sampler         CreateSampler(const SamplerCreation& samplerCreation) override;
        PipelineState   CreateGraphicsPipelineState(const GraphicsPipelineCreation& graphicsPipelineCreation) override;
        PipelineState   CreateComputePipelineState(const ComputePipelineCreation& computePipelineCreation) override;
        BindingSet&     CreateBindingSet(RID shader, const BindingSetType& bindingSetType) override;
        void            DestroySwapchain(const Swapchain& swapchain) override;
        void            DestroyRenderPass(const RenderPass& renderPass) override;
        void            DestroyBuffer(const Buffer& buffer) override;
        void            DestroyTexture(const Texture& texture) override;
        void            DestroyTextureView(const TextureView& textureView) override;
        void            DestroySampler(const Sampler& sampler) override;
        void            DestroyGraphicsPipelineState(const PipelineState& pipelineState) override;
        void            DestroyComputePipelineState(const PipelineState& pipelineState) override;
        void            DestroyBindingSet(BindingSet& bindingSet) override;
        RenderCommands& BeginFrame() override;
        RenderPass      AcquireNextRenderPass(Swapchain swapchain) override;
        void            EndFrame(Swapchain swapchain) override;
        void            WaitQueue() override;
        void            UpdateBufferData(const BufferDataInfo& bufferDataInfo) override;


        bool CreateSwapchain(VulkanSwapchain* vulkanSwapchain);
        void DestroySwapchain(VulkanSwapchain* vulkanSwapchain);

        void    ImGuiInit(Swapchain renderSwapchain) override;
        void    ImGuiNewFrame() override;
        void    ImGuiRender(RenderCommands& renderCommands) override;
        VoidPtr GetImGuiTexture(const Texture& texture) override;
    };


    SharedPtr<RenderDevice> CreateVulkanDevice();
}
