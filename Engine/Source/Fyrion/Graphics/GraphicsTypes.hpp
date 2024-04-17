#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    FY_HANDLER(Adapter);
    FY_HANDLER(Swapchain);
    FY_HANDLER(RenderPass);
    FY_HANDLER(PipelineState);
    FY_HANDLER(Texture);
    FY_HANDLER(TextureView);
    FY_HANDLER(Buffer);
    FY_HANDLER(Sampler);

    enum class ImageFormat
    {
        R,
        R16F,
        R32F,
        RG,
        RG16F,
        RG32F,
        RGB,
        RGB16F,
        RGB32F,
        RGBA,
        RGBA16F,
        RGBA32F,
        BGRA,
        Depth,
        Undefined,
        //TODO : add ohter formats
    };

    enum class RenderApiType
    {
        None   = 0,
        Vulkan = 1,
        OpenGL = 2,
        D3D12  = 3,
        Metal  = 4,
        WebGPU = 5
    };

    enum class ResourceLayout
    {
        Undefined              = 0,
        General                = 1,
        ColorAttachment        = 2,
        DepthStencilAttachment = 3,
        ShaderReadOnly         = 4,
        CopyDest               = 5,
        CopySource             = 6,
        Present                = 7
    };

    enum class ViewType
    {
        Type1D          = 0,
        Type2D          = 1,
        Type3D          = 2,
        TypeCube        = 3,
        Type1DArray     = 4,
        Type2DArray     = 5,
        TypeCubeArray   = 6,
        Undefined       = 7,
    };

    enum class ShaderStage : u32
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
    ENUM_FLAGS(ShaderStage, u32)

    enum class BufferUsage : u32
    {
        VertexBuffer                 = 1 << 0,
        IndexBuffer                  = 1 << 2,
        UniformBuffer                = 1 << 3,
        StorageBuffer                = 1 << 4,
        IndirectBuffer               = 1 << 5,
        AccelerationStructureBuild   = 1 << 6,
        AccelerationStructureStorage = 1 << 7,
        All                          = VertexBuffer | IndexBuffer | UniformBuffer | StorageBuffer | IndirectBuffer | AccelerationStructureBuild | AccelerationStructureStorage
    };

    ENUM_FLAGS(BufferUsage, u32)

    enum class BufferMemory
    {
        GPUOnly       = 1,
        TransferToGPU = 2,
        TransferToCPU = 3,
        Dynamic       = 4
    };

    enum class TextureUsage : u32
    {
        None           = 0 << 0,
        ShaderResource = 1 << 0,
        DepthStencil   = 1 << 2,
        RenderPass     = 1 << 3,
        Storage        = 1 << 4,
        TransferDst    = 1 << 5,
        TransferSrc    = 1 << 6,
    };

    ENUM_FLAGS(TextureUsage, u32)

    enum class SamplerFilter
    {
        Nearest  = 0,
        Linear   = 1,
        CubicImg = 2,
    };

    enum class TextureAddressMode
    {
        Repeat            = 0,
        MirroredRepeat    = 1,
        ClampToEdge       = 2,
        ClampToBorder     = 3,
        MirrorClampToEdge = 4,
    };

    enum class CompareOp
    {
        Never          = 0,
        Less           = 1,
        Equal          = 2,
        LessOrEqual    = 3,
        Greater        = 4,
        NotEqual       = 5,
        GreaterOrEqual = 6,
        Always         = 7
    };

    enum class BorderColor
    {
        FloatTransparentBlack = 0,
        IntTransparentBlack   = 1,
        FloatOpaqueBlack      = 2,
        IntOpaqueBlack        = 3,
        FloatOpaqueWhite      = 4,
        IntOpaqueWhite        = 4,
    };


    struct SwapchainCreation
    {
        Window window{};
        bool vsync{true};
    };

    struct BufferCreation
    {
        BufferUsage  usage{};
        usize        size{};
        BufferMemory memory{BufferMemory::Dynamic};
    };

    struct TextureCreation
    {
        Extent3D     extent{};
        ImageFormat  format{ImageFormat::RGBA};
        TextureUsage usage{};
        u32          mipLevels{1};
        u32          arrayLayers{1};
        ViewType     defaultView{ViewType::Undefined};
    };

    struct TextureViewCreation
    {
        Texture  texture{};
        ViewType    viewType{ViewType::Type2D};
        u32         baseMipLevel = 0;
        u32         levelCount = 1;
        u32         baseArrayLayer = 0;
        u32         layerCount = 1;
    };

    struct SamplerCreation
    {
        SamplerFilter      filter{SamplerFilter::Linear};
        TextureAddressMode addressMode{TextureAddressMode::Repeat};
        CompareOp          compareOperator{};
        f32                minLod{};
        f32                maxLod{};
        BorderColor        borderColor{BorderColor::IntOpaqueBlack};
    };

    //TODO add other fields.
    struct GraphicsPipelineCreation
    {
        RID shader{};
    };

    struct ComputePipelineCreation
    {
        RID shader{};
    };


    struct ClearDepthStencilValue
    {
        f32 depth{1.0};
        u32 stencil{0};
    };

    struct BeginRenderPassInfo
    {
        RenderPass          renderPass{};
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

    struct BindingSet
    {
        //TODO
    };

    struct RenderCommands
    {
        virtual ~RenderCommands() = default;
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void BeginRenderPass(const BeginRenderPassInfo& beginRenderPassInfo) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SetViewport(const ViewportInfo& viewportInfo) = 0;
        virtual void BindVertexBuffer(const Buffer& gpuBuffer) = 0;
        virtual void BindIndexBuffer(const Buffer& gpuBuffer) = 0;
        virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) = 0;
        virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) = 0;
        virtual void PushConstants(const PipelineState& pipeline, ShaderStage stages, const void* data, usize size) = 0;
        virtual void BindBindingSet(const PipelineState& pipeline, const BindingSet& bindingSet) = 0;
        virtual void DrawIndexedIndirect(const Buffer& buffer, usize offset, u32 drawCount, u32 stride) = 0;
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
