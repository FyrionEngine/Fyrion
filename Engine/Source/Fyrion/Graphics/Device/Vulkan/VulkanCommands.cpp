#include "VulkanCommands.hpp"

#include "VulkanBindingSet.hpp"
#include "VulkanDevice.hpp"
#include "VulkanUtils.hpp"

namespace Fyrion
{
    VulkanCommands::VulkanCommands(VulkanDevice& vulkanDevice) : vulkanDevice(vulkanDevice)
    {
        VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = vulkanDevice.graphicsFamily;
        vkCreateCommandPool(vulkanDevice.device, &commandPoolInfo, nullptr, &commandPool);

        VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(vulkanDevice.device, &allocInfo, &commandBuffer);
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

        bool shouldClear = false;

        if (beginRenderPassInfo.clearValue)
        {
            i32 colorClearValuesCount = vulkanRenderPass->hasDepth ? vulkanRenderPass->clearValues.Size() - 1 : vulkanRenderPass->clearValues.Size();
            for (int i = 0; i < colorClearValuesCount ; ++i)
            {
                shouldClear = true;
                VkClearValue& clearValue = vulkanRenderPass->clearValues[i];
                Vec4 color = *beginRenderPassInfo.clearValue;
                clearValue.color = {color.x, color.y, color.z, color.w};
            }
        }

        if (beginRenderPassInfo.depthStencil && vulkanRenderPass->hasDepth)
        {
            shouldClear = true;
            VkClearValue& clearValue = vulkanRenderPass->clearValues[vulkanRenderPass->clearValues.Size() - 1];
            clearValue.depthStencil = {
                .depth = beginRenderPassInfo.depthStencil->depth,
                .stencil = beginRenderPassInfo.depthStencil->stencil
            };
        }

        if (shouldClear)
        {
            renderPassBeginInfo.clearValueCount = vulkanRenderPass->clearValues.Size();
            renderPassBeginInfo.pClearValues = vulkanRenderPass->clearValues.Data();
        }

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
        VkBuffer     vertexBuffers[] = {static_cast<const VulkanBuffer*>(gpuBuffer.handler)->buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }

    void VulkanCommands::BindIndexBuffer(const Buffer& gpuBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, static_cast<const VulkanBuffer*>(gpuBuffer.handler)->buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommands::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanCommands::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommands::PushConstants(const PipelineState& pipeline, ShaderStage stages, const void* data, usize size)
    {
        VulkanPipelineState& pipelineState = *static_cast<VulkanPipelineState*>(pipeline.handler);
        vkCmdPushConstants(commandBuffer, pipelineState.layout, Vulkan::CastStage(stages), 0, size, data);
    }

    void VulkanCommands::BindBindingSet(const PipelineState& pipeline, BindingSet* bindingSet)
    {
        VulkanBindingSet* vulkanBindingSet = static_cast<VulkanBindingSet*>(bindingSet);
        vulkanBindingSet->Bind(*this,pipeline);
    }

    void VulkanCommands::DrawIndexedIndirect(const Buffer& buffer, usize offset, u32 drawCount, u32 stride)
    {
        const VulkanBuffer& vulkanBuffer = *static_cast<const VulkanBuffer*>(buffer.handler);
        vkCmdDrawIndexedIndirect(commandBuffer, vulkanBuffer.buffer, offset, drawCount, stride);
    }

    void VulkanCommands::BindPipelineState(const PipelineState& pipeline)
    {
        auto& pipelineState = *static_cast<VulkanPipelineState*>(pipeline.handler);
        vkCmdBindPipeline(commandBuffer, pipelineState.bindingPoint, pipelineState.pipeline);
    }

    void VulkanCommands::Dispatch(u32 x, u32 y, u32 z)
    {
        vkCmdDispatch(commandBuffer, Math::Max(x, 1u), Math::Max(y, 1u), Math::Max(z, 1u));
    }

    void VulkanCommands::TraceRays(PipelineState pipeline, u32 x, u32 y, u32 z)
    {

    }

    void VulkanCommands::BeginLabel(const StringView& name, const Vec4& color)
    {
        if (vulkanDevice.validationLayersAvailable)
        {
            VkDebugUtilsLabelEXT vkDebugUtilsLabelExt{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
            vkDebugUtilsLabelExt.pLabelName = name.CStr();
            vkDebugUtilsLabelExt.color[0] = color.r;
            vkDebugUtilsLabelExt.color[1] = color.g;
            vkDebugUtilsLabelExt.color[2] = color.b;
            vkDebugUtilsLabelExt.color[3] = color.a;
            vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &vkDebugUtilsLabelExt);
        }
    }

    void VulkanCommands::EndLabel()
    {
        if (vulkanDevice.validationLayersAvailable)
        {
            vkCmdEndDebugUtilsLabelEXT(commandBuffer);
        }
    }

    void VulkanCommands::ResourceBarrier(const ResourceBarrierInfo& resourceBarrierInfo)
    {
        VkImageSubresourceRange subresourceRange = {};
        if (resourceBarrierInfo.isDepth)
        {
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else
        {
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        auto vulkanTexture = static_cast<VulkanTexture*>(resourceBarrierInfo.texture.handler);

        subresourceRange.baseMipLevel = resourceBarrierInfo.mipLevel;
        subresourceRange.levelCount = Math::Max(resourceBarrierInfo.levelCount, 1u);
        subresourceRange.layerCount = Math::Max(resourceBarrierInfo.layerCount, 1u);
        subresourceRange.baseArrayLayer = resourceBarrierInfo.baseArrayLayer;

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange = subresourceRange;
        barrier.oldLayout = Vulkan::CastLayout(resourceBarrierInfo.oldLayout);
        barrier.newLayout = Vulkan::CastLayout(resourceBarrierInfo.newLayout);
        barrier.image = vulkanTexture->image;

        switch (barrier.oldLayout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_GENERAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        switch (barrier.newLayout)
        {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_GENERAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                if (barrier.srcAccessMask == 0)
                {
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
    }

    void VulkanCommands::CopyBuffer(Buffer srcBuffer, Buffer dstBuffer, const Span<BufferCopyInfo>& info)
    {
        vkCmdCopyBuffer(commandBuffer,
                        static_cast<VulkanBuffer*>(srcBuffer.handler)->buffer,
                        static_cast<VulkanBuffer*>(dstBuffer.handler)->buffer,
                        info.Size(),
                        (const VkBufferCopy*)info.Data()
        );
    }

    void VulkanCommands::CopyBufferToTexture(Buffer srcBuffer, Texture texture, const Span<BufferImageCopy>& regions)
    {
        Array<VkBufferImageCopy> vkBufferImageCopies(regions.Size());
        for (usize i = 0; i < regions.Size(); ++i)
        {
            VkBufferImageCopy& dest = vkBufferImageCopies[i];
            const BufferImageCopy& src = regions[i];

            dest.bufferOffset = src.bufferOffset;
            dest.bufferRowLength = src.bufferRowLength;
            dest.bufferImageHeight = src.bufferImageHeight;
            dest.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            dest.imageSubresource.layerCount = src.layerCount;
            dest.imageSubresource.mipLevel = src.textureMipLevel;
            dest.imageSubresource.baseArrayLayer = src.textureArrayLayer;
            dest.imageOffset = {src.imageOffset.x, src.imageOffset.y, src.imageOffset.z};
            dest.imageExtent = {src.imageExtent.width,  src.imageExtent.height, src.imageExtent.depth};
        }

        vkCmdCopyBufferToImage(commandBuffer,
                               static_cast<VulkanBuffer*>(srcBuffer.handler)->buffer,
                               static_cast<VulkanTexture*>(texture.handler)->image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               vkBufferImageCopies.Size(),
                               vkBufferImageCopies.Data());
    }

    void VulkanCommands::CopyTextureToBuffer(Texture srcTexture, ResourceLayout textureLayout, Buffer destBuffer, const Span<BufferImageCopy>& regions)
    {
        Array<VkBufferImageCopy> vkBufferImageCopies(regions.Size());
        for (usize i = 0; i < regions.Size(); ++i)
        {
            VkBufferImageCopy& dest = vkBufferImageCopies[i];
            const BufferImageCopy& src = regions[i];

            dest.bufferOffset = src.bufferOffset;
            dest.bufferRowLength = src.bufferRowLength;
            dest.bufferImageHeight = src.bufferImageHeight;
            dest.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            dest.imageSubresource.layerCount = src.layerCount;
            dest.imageSubresource.mipLevel = src.textureMipLevel;
            dest.imageSubresource.baseArrayLayer = src.textureArrayLayer;
            dest.imageOffset = {src.imageOffset.x, src.imageOffset.y, src.imageOffset.z};
            dest.imageExtent = {src.imageExtent.width,  src.imageExtent.height, src.imageExtent.depth};
        }

        vkCmdCopyImageToBuffer(commandBuffer,
                               static_cast<VulkanTexture*>(srcTexture.handler)->image,
                               Vulkan::CastLayout(textureLayout),
                               static_cast<VulkanBuffer*>(destBuffer.handler)->buffer,
                               vkBufferImageCopies.Size(),
                               vkBufferImageCopies.Data());


    }

    void VulkanCommands::CopyTexture(Texture srcTexture, ResourceLayout srcTextureLayout, Texture dstTexture, ResourceLayout dstTextureLayout, const Span<TextureCopy>& regions)
    {
        Array<VkImageCopy> vkImageCopies;
        vkImageCopies.Reserve(regions.Size());

        for (const TextureCopy& textureCopy : regions)
        {
            vkImageCopies.EmplaceBack(VkImageCopy{
                .srcSubresource = {
                    .aspectMask = Vulkan::CastTextureAspect(textureCopy.srcSubresource.textureAspect),
                    .mipLevel = textureCopy.srcSubresource.mipLevel,
                    .baseArrayLayer = textureCopy.srcSubresource.baseArrayLayer,
                    .layerCount = textureCopy.srcSubresource.layerCount
                },
                .srcOffset = {textureCopy.srcOffset.x, textureCopy.srcOffset.y, textureCopy.srcOffset.z},
                .dstSubresource = {
                    .aspectMask = Vulkan::CastTextureAspect(textureCopy.dstSubresource.textureAspect),
                    .mipLevel = textureCopy.dstSubresource.mipLevel,
                    .baseArrayLayer = textureCopy.dstSubresource.baseArrayLayer,
                    .layerCount = textureCopy.dstSubresource.layerCount
                },
                .dstOffset = {textureCopy.dstOffset.x, textureCopy.dstOffset.y, textureCopy.dstOffset.z},
                .extent = {textureCopy.extent.width, textureCopy.extent.height, textureCopy.extent.depth},
            });
        }

        vkCmdCopyImage(commandBuffer,
                       static_cast<VulkanTexture*>(srcTexture.handler)->image,
                       Vulkan::CastLayout(srcTextureLayout),
                       static_cast<VulkanTexture*>(dstTexture.handler)->image,
                       Vulkan::CastLayout(dstTextureLayout),
                       vkImageCopies.Size(),
                       vkImageCopies.Data());
    }


    void VulkanCommands::SubmitAndWait(GPUQueue queue)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkQueue vkQueue = static_cast<VkQueue>(queue.handler);

        vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(vkQueue);
    }
}
