#include "VulkanCommands.hpp"
#include "VulkanDevice.hpp"

namespace Fyrion
{

    VulkanCommands::VulkanCommands(VulkanDevice& vulkanDevice) : vulkanDevice(vulkanDevice)
    {
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
