#include "VulkanCommands.hpp"
#include "VulkanDevice.hpp"

namespace Fyrion
{

    VulkanCommands::VulkanCommands(VulkanDevice& vulkanDevice) : vulkanDevice(vulkanDevice)
    {
        VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = vulkanDevice.graphicsFamily;
        vkCreateCommandPool(vulkanDevice.device, &commandPoolInfo, nullptr, &commandPool);

        VkCommandBufferAllocateInfo tempAllocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        tempAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        tempAllocInfo.commandPool = commandPool;
        tempAllocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(vulkanDevice.device, &tempAllocInfo, &commandBuffer);

    }

    void VulkanCommands::BeginRenderPass(const BeginRenderPassInfo& beginRenderPassInfo)
    {

    }

    void VulkanCommands::EndRenderPass()
    {

    }

    void VulkanCommands::SetViewport(const ViewportInfo& viewportInfo)
    {

    }

    void VulkanCommands::BindVertexBuffer(const GPUBuffer& gpuBuffer)
    {

    }

    void VulkanCommands::BindIndexBuffer(const GPUBuffer& gpuBuffer)
    {

    }

    void VulkanCommands::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
    {

    }

    void VulkanCommands::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {

    }

    void VulkanCommands::PushConstants(const PipelineState& pipeline, ShaderStage stages, const void* data, usize size)
    {

    }

    void VulkanCommands::BindBindingSet(const PipelineState& pipeline, const BindingSet& bindingSet)
    {

    }

    void VulkanCommands::DrawIndexedIndirect(const GPUBuffer& buffer, usize offset, u32 drawCount, u32 stride)
    {

    }

    void VulkanCommands::BindPipelineState(const PipelineState& pipeline)
    {

    }

    void VulkanCommands::Dispatch(u32 x, u32 y, u32 z)
    {

    }

    void VulkanCommands::TraceRays(PipelineState pipeline, u32 x, u32 y, u32 z)
    {

    }

    void VulkanCommands::SetScissor(const Rect& rect)
    {

    }

    void VulkanCommands::BeginLabel(const StringView& name, const Vec4& color)
    {

    }

    void VulkanCommands::EndLabel()
    {

    }

    void VulkanCommands::ResourceBarrier(const ResourceBarrierInfo& resourceBarrierInfo)
    {

    }
}
