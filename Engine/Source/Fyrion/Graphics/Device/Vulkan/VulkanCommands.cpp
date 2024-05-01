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

    void VulkanCommands::Begin()
    {
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

    void VulkanCommands::End()
    {
        vkEndCommandBuffer(commandBuffer);
    }

    void VulkanCommands::BeginRenderPass(const BeginRenderPassInfo& beginRenderPassInfo)
    {
        VulkanRenderPass* vulkanRenderPass = static_cast<VulkanRenderPass*>(beginRenderPassInfo.renderPass.handler);

        VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderPassBeginInfo.renderPass = vulkanRenderPass->renderPass;
        renderPassBeginInfo.framebuffer = vulkanRenderPass->framebuffer;
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = {vulkanRenderPass->extent.width, vulkanRenderPass->extent.height};

        for (int i = 0; i < vulkanRenderPass->clearValues.Size(); ++i)
        {
            VkClearValue& clearValue = vulkanRenderPass->clearValues[i];
            if (beginRenderPassInfo.clearValues.Size() > i)
            {
                Vec4 color = beginRenderPassInfo.clearValues[i];
                clearValue.color = {color.x, color.y, color.z, color.w};
            }
            else
            {
                clearValue.depthStencil = {.depth = beginRenderPassInfo.depthStencil.depth, .stencil = beginRenderPassInfo.depthStencil.stencil};
            }
        }

        renderPassBeginInfo.clearValueCount = vulkanRenderPass->clearValues.Size();
        renderPassBeginInfo.pClearValues = vulkanRenderPass->clearValues.Data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommands::EndRenderPass()
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanCommands::SetViewport(const ViewportInfo& viewportInfo)
    {
        VkViewport vkViewport;
        vkViewport.x = viewportInfo.x;
        vkViewport.y = viewportInfo.y;
        vkViewport.width = viewportInfo.width;
        vkViewport.height = viewportInfo.height;
        vkViewport.minDepth = viewportInfo.minDepth;
        vkViewport.maxDepth = viewportInfo.maxDepth;
        vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
    }

    void VulkanCommands::SetScissor(const Rect& rect)
    {
        VkRect2D rect2D;
        rect2D.offset.x = static_cast<i32>(rect.x);
        rect2D.offset.y = static_cast<i32>(rect.y);
        rect2D.extent.width = static_cast<u32>(rect.width);
        rect2D.extent.height = static_cast<u32>(rect.height);
        vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
    }


    void VulkanCommands::BindVertexBuffer(const Buffer& gpuBuffer)
    {

    }

    void VulkanCommands::BindIndexBuffer(const Buffer& gpuBuffer)
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

    void VulkanCommands::DrawIndexedIndirect(const Buffer& buffer, usize offset, u32 drawCount, u32 stride)
    {

    }

    void VulkanCommands::BindPipelineState(const PipelineState& pipeline)
    {
        auto& pipelineState =  *static_cast<VulkanPipelineState*>(pipeline.handler);
        vkCmdBindPipeline(commandBuffer, pipelineState.bindingPoint, pipelineState.pipeline);
    }

    void VulkanCommands::Dispatch(u32 x, u32 y, u32 z)
    {

    }

    void VulkanCommands::TraceRays(PipelineState pipeline, u32 x, u32 y, u32 z)
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
