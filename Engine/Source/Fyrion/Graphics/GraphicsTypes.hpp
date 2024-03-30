#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    struct BindingSet;

    enum class RenderApiType
    {
        None = 0,
        Vulkan = 1,
        OpenGL = 2,
        D3D12 = 3,
        Metal = 4,
        WebGPU = 5
    };

    enum class ResourceLayout
    {
        Undefined = 0,
        General = 1,
        ColorAttachment = 2,
        DepthStencilAttachment = 3,
        ShaderReadOnly = 4,
        CopyDest = 5,
        CopySource = 6,
        Present = 7
    };

    enum class ShaderStage
    {
        Vertex         = 1 << 0,
        TessControl    = 1 << 1,
        TessEvaluation = 1 << 2,
        Geometry       = 1 << 3,
        Fragment       = 1 << 4,
        Compute        = 1 << 5,
        Raygen         = 1 << 6,
        Intersect      = 1 << 7,
        AnyHit         = 1 << 8,
        ClosestHit     = 1 << 9,
        Miss           = 1 << 10,
        Callable       = 1 << 11,
        All            = 1 << 12,
    };
    ENUM_FLAGS(ShaderStage, i32)

    FY_HANDLER(GPUAdapter);
    FY_HANDLER(Swapchain);
    FY_HANDLER(RenderPass);
    FY_HANDLER(PipelineState);
    FY_HANDLER(Texture);
    FY_HANDLER(TextureView);
    FY_HANDLER(GPUBuffer);
    FY_HANDLER(Sampler);

    struct SwapchainCreation
    {
        Window window{};
        bool vsync{true};
    };

    struct ClearDepthStencilValue
    {
        f32 depth{1.0};
        u32 stencil{0};
    };

    struct BeginRenderPassInfo
    {
        RenderPass             renderPass{};
        Span<Vec4>             clearValues{};
        ClearDepthStencilValue depthStencil{};
    };

    struct ViewportInfo
    {
        f32 x{};
        f32 y{};
        f32 width{};
        f32 height{};
        f32 minDepth{};
        f32 maxDepth{};
    };

    struct ResourceBarrierInfo
    {
        Texture texture{};
        ResourceLayout oldLayout{};
        ResourceLayout newLayout{};
        u32 mipLevel{0};
        u32 levelCount{0};
        u32 baseArrayLayer{0};
        u32 layerCount{0};
        bool isDepth{false};
    };

    struct RenderCommands
    {
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void BeginRenderPass(const BeginRenderPassInfo& beginRenderPassInfo) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SetViewport(const ViewportInfo& viewportInfo) = 0;
        virtual void BindVertexBuffer(const GPUBuffer& gpuBuffer) = 0;
        virtual void BindIndexBuffer(const GPUBuffer& gpuBuffer) = 0;
        virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) = 0;
        virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) = 0;
        virtual void PushConstants(const PipelineState& pipeline, ShaderStage stages, const void* data, usize size) = 0;
        virtual void BindBindingSet(const PipelineState& pipeline, const BindingSet& bindingSet) = 0;
        virtual void DrawIndexedIndirect(const GPUBuffer& buffer, usize offset, u32 drawCount, u32 stride) = 0;
        virtual void BindPipelineState(const PipelineState& pipeline) = 0;
        virtual void Dispatch(u32 x, u32 y, u32 z) = 0;
        virtual void TraceRays(PipelineState pipeline, u32 x, u32 y, u32 z) = 0;
        virtual void SetScissor(const Rect& rect) = 0;
        virtual void BeginLabel(const StringView& name, const Vec4& color) = 0;
        virtual void EndLabel() = 0;
        virtual void ResourceBarrier(const ResourceBarrierInfo& resourceBarrierInfo) = 0;
    };

    struct DeviceFeatures
    {
        bool raytraceSupported{};
        bool bindlessSupported{};
        bool multiDrawIndirectSupported{};
    };



}