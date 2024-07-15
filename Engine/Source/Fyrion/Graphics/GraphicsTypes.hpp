#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    class ShaderAsset;

    FY_HANDLER(Adapter);
    FY_HANDLER(Swapchain);
    FY_HANDLER(RenderPass);
    FY_HANDLER(PipelineState);
    FY_HANDLER(Texture);
    FY_HANDLER(TextureView);
    FY_HANDLER(Buffer);
    FY_HANDLER(Sampler);
    FY_HANDLER(GPUQueue);

    enum class Format
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
        Unknown         = 0,
        Vertex          = 1 << 0,
        Hull            = 1 << 1,
        Domain          = 1 << 2,
        Geometry        = 1 << 3,
        Pixel           = 1 << 4,
        Compute         = 1 << 5,
        Amplification   = 1 << 6,
        Mesh            = 1 << 7,
        RayGen          = 1 << 8,
        RayMiss         = 1 << 9,
        RayClosestHit   = 1 << 10,
        RayAnyHit       = 1 << 11,
        RayIntersection = 1 << 12,
        Callable        = 1 << 13,
        All             = 1 << 14
    };

    ENUM_FLAGS(ShaderStage, u32)

    enum class BufferUsage : u32
    {
        None                         = 1 << 0,
        VertexBuffer                 = 1 << 1,
        IndexBuffer                  = 1 << 2,
        UniformBuffer                = 1 << 3,
        StorageBuffer                = 1 << 4,
        IndirectBuffer               = 1 << 5,
        AccelerationStructureBuild   = 1 << 6,
        AccelerationStructureStorage = 1 << 7,
        All                          = VertexBuffer | IndexBuffer | UniformBuffer | StorageBuffer | IndirectBuffer | AccelerationStructureBuild | AccelerationStructureStorage
    };

    ENUM_FLAGS(BufferUsage, u32)

    enum class BufferAllocation
    {
        GPUOnly       = 1,
        TransferToGPU = 2,
        TransferToCPU = 3
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

    enum class LoadOp
    {
        Load = 0,
        Clear = 1,
        DontCare = 2
    };

    enum class StoreOp
    {
        Store = 0,
        DontCare = 1
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

    enum class CullMode
    {
        None  = 0,
        Front = 1,
        Back  = 2
    };


    enum class PolygonMode
    {
        Fill  = 0,
        Line  = 1,
        Point = 2,
    };

    enum class PrimitiveTopology
    {
        PointList                  = 0,
        LineList                   = 1,
        LineStrip                  = 2,
        TriangleList               = 3,
        TriangleStrip              = 4,
        TriangleFan                = 5,
        LineListWithAdjacency      = 6,
        LineStripWithAdjacency     = 7,
        TriangleListWithAdjacency  = 8,
        TriangleStripWithAdjacency = 9,
        PatchList                  = 10,
    };

    enum class DescriptorType
    {
        SampledImage          = 0,
        Sampler               = 1,
        StorageImage          = 2,
        UniformBuffer         = 3,
        StorageBuffer         = 4,
        AccelerationStructure = 5
    };

    enum class RenderType
    {
        None,
        Void,
        Bool,
        Int,
        Float,
        Vector,
        Matrix,
        Image,
        Sampler,
        SampledImage,
        Array,
        RuntimeArray,
        Struct
    };

    enum class SamplerMipmapMode
    {
        Nearest,
        Linear
    };

    enum class RenderGraphResourceType
    {
        Buffer     = 0,
        Texture    = 1,
        Attachment = 2,
        Reference  = 3,
    };

    enum class RenderGraphPassType
    {
        Other    = 0,
        Graphics = 1,
        Compute  = 2
    };

    enum class AlphaMode
    {
        None        = 0,
        Opaque      = 1,
        Mask        = 2,
        Blend       = 3
    };

    struct SwapchainCreation
    {
        Window window{};
        bool vsync{true};
    };

    struct AttachmentCreation
    {
        Texture        texture{};
        ResourceLayout initialLayout{ResourceLayout::Undefined};
        ResourceLayout finalLayout{ResourceLayout::Undefined};
        LoadOp         loadOp{LoadOp::Clear};
        StoreOp        storeOp{StoreOp::Store};
    };

    struct RenderPassCreation
    {
        Span<AttachmentCreation> attachments{};
    };

    struct BufferCreation
    {
        BufferUsage      usage{BufferUsage::None};
        usize            size{};
        BufferAllocation allocation{BufferAllocation::GPUOnly};
    };

    struct TextureCreation
    {
        Extent3D     extent{};
        Format       format{Format::RGBA};
        TextureUsage usage{};
        u32          mipLevels{1};
        u32          arrayLayers{1};
        ViewType     defaultView{ViewType::Type2D};
    };

    struct TextureViewCreation
    {
        Texture  texture{};
        ViewType viewType{ViewType::Type2D};
        u32      baseMipLevel = 0;
        u32      levelCount = 1;
        u32      baseArrayLayer = 0;
        u32      layerCount = 1;
    };

    struct SamplerCreation
    {
        SamplerFilter      filter{SamplerFilter::Linear};
        TextureAddressMode addressMode{TextureAddressMode::Repeat};
        CompareOp          compareOperator{CompareOp::Always};
        f32                mipLodBias = 0.0f;
        f32                minLod{};
        f32                maxLod{};
        bool               anisotropyEnable = true;
        BorderColor        borderColor{BorderColor::IntOpaqueBlack};
        SamplerMipmapMode  samplerMipmapMode = SamplerMipmapMode::Linear;
    };

    struct ShaderCreation
    {
        StringView    source{};
        StringView    entryPoint{};
        ShaderStage   shaderStage{};
        RenderApiType renderApi{};
    };

    struct GraphicsPipelineCreation
    {
        ShaderAsset*      shader{};
        Span<Format>      attachments{};
        RenderPass        renderPass{};
        Format            depthFormat = Format::Undefined;
        bool              depthWrite{false};
        bool              stencilTest{false};
        bool              blendEnabled{false};
        f32               minDepthBounds{1.0};
        f32               maxDepthBounds{0.0};
        CullMode          cullMode{CullMode::None};
        CompareOp         compareOperator{CompareOp::Always};
        PolygonMode       polygonMode{PolygonMode::Fill};
        PrimitiveTopology primitiveTopology{PrimitiveTopology::TriangleList};
        PipelineState     pipelineState{};
    };

    struct ComputePipelineCreation
    {
        //RID shader{};
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

    struct InterfaceVariable
    {
        u32    location{};
        u32    offset{};
        String name{};
        Format format{};
        u32    size{};
    };

    struct TypeDescription
    {
        String                 name{};
        RenderType             type{};
        u32                    size{};
        u32                    offset{};
        Array<TypeDescription> members{};
    };

    struct DescriptorBinding
    {
        u32                    binding{};
        u32                    count{};
        String                 name{};
        DescriptorType         descriptorType{};
        RenderType             renderType{};
        ShaderStage            shaderStage{ShaderStage::All};
        ViewType               viewType{};
        Array<TypeDescription> members{};
        u32                    size{};
    };

    struct DescriptorLayout
    {
        u32                      set{};
        Array<DescriptorBinding> bindings{};
    };

    struct ShaderPushConstant
    {
        String      name{};
        u32         offset{};
        u32         size{};
        ShaderStage stage{};
    };

    struct ShaderStageInfo
    {
        ShaderStage stage{};
        String      entryPoint{};
        u32         offset{};
        u32         size{};
    };

    struct ShaderInfo
    {
        Array<InterfaceVariable>  inputVariables{};
        Array<InterfaceVariable>  outputVariables{};
        Array<DescriptorLayout>   descriptors{};
        Array<ShaderPushConstant> pushConstants{};
        u32                       stride{};
    };

    struct BufferDataInfo
    {
        Buffer      buffer{};
        const void* data{};
        usize       size{};
        usize       offset{};
    };

    struct BufferImageCopy
    {
        u64      bufferOffset{};
        u32      bufferRowLength{};
        u32      bufferImageHeight{};
        u32      textureMipLevel{};
        u32      textureArrayLayer{};
        u32      layerCount{1};
        Offset3D imageOffset{};
        Extent3D imageExtent;
    };


    struct TextureDataRegion
    {
        usize    dataOffset{};
        u32      layerCount{};
        u32      mipLevel{};
        u32      arrayLayer{};
        u32      levelCount{};
        Extent3D extent{};
    };

    struct TextureDataInfo
    {
        Texture                 texture{};
        const u8*               data{nullptr};
        usize                   size{};
        Extent3D                extent{};
        Span<TextureDataRegion> regions{};
    };

    struct VertexData final
    {
        Vec3 position{};
        Vec3 normal{};
        Vec3 color{};
        Vec2 uv{};
        Vec4 tangent{};
    };

    inline bool operator==(const VertexData& r, const VertexData& l)
    {
        return r.position == l.position && r.normal == l.normal && r.uv == l.uv && r.color == l.color && r.tangent == l.tangent;
    }

    template<>
    struct Hash<VertexData>
    {
        static constexpr bool hasHash = true;
        static usize Value(const VertexData& value)
        {
            return (Hash<Vec3>::Value(value.position) ^ Hash<Vec3>::Value(value.normal) << 1) >> 1 ^ Hash<Vec2>::Value(value.uv) << 1;
        }
    };

    struct MeshPrimitive final
    {
        u32 firstIndex{};
        u32 indexCount{};
        u32 materialIndex{};
    };


    struct BufferCopyInfo
    {
        usize srcOffset;
        usize dstOffset;
        usize size;
    };

    struct BindingVar;


    template<typename T>
    struct BindingValueSetter
    {
        static void SetValue(BindingVar& bindingVar, const T& value);
    };

    struct FY_API BindingVar
    {
        virtual ~BindingVar() = default;

        template<typename T>
        void Set(const T& val)
        {
            BindingValueSetter<T>::SetValue(*this, val);
        }

        virtual void SetTexture(const Texture& texture) = 0;
        virtual void SetTextureView(const TextureView& textureView) = 0;
        virtual void SetSampler(const Sampler& sampler) = 0;
        virtual void SetBuffer(const Buffer& buffer) = 0;
        virtual void SetValue(ConstPtr ptr, usize size) = 0;
    };

    template <typename T>
    void BindingValueSetter<T>::SetValue(BindingVar& bindingVar, const T& value)
    {
        bindingVar.SetValue(&value, sizeof(T));
    }

    struct FY_API BindingSet
    {
        virtual ~BindingSet() = default;

        virtual BindingVar* GetVar(const StringView& name) = 0;
    };

    template<>
    struct BindingValueSetter<Texture>
    {
        static void SetValue(BindingVar& bindingVar, const Texture& value)
        {
            bindingVar.SetTexture(value);
        }
    };

    template<>
    struct BindingValueSetter<TextureView>
    {
        static void SetValue(BindingVar& bindingVar, const TextureView& value)
        {
            bindingVar.SetTextureView(value);
        }
    };

    template<>
    struct BindingValueSetter<Sampler>
    {
        static void SetValue(BindingVar& bindingVar, const Sampler& value)
        {
            bindingVar.SetSampler(value);
        }
    };

    template<>
    struct BindingValueSetter<Buffer>
    {
        static void SetValue(BindingVar& bindingVar, const Buffer& value)
        {
            bindingVar.SetBuffer(value);
        }
    };

    struct RenderGraphEdge
    {
        String output{};
        String nodeOutput{};
        String input{};
        String nodeInput{};

        static void RegisterType(NativeTypeHandler<RenderGraphEdge>& type);
    };


    struct RenderGraphResourceCreation
    {
        String                  name{};
        RenderGraphResourceType type{};
        Extent3D                size{};
        Vec2                    scale{};
        LoadOp                  loadOp{LoadOp::Clear};
        Vec4                    cleanValue{};
        bool                    cleanDepth{};
        Format                  format{};
        BufferUsage             bufferUsage{};
        BufferAllocation        bufferAllocation{BufferAllocation::GPUOnly};
        usize                   bufferInitialSize{};
    };

    struct RenderGraphPassCreation
    {
        String                             name{};
        Array<RenderGraphResourceCreation> inputs{};
        Array<RenderGraphResourceCreation> outputs{};
        RenderGraphPassType                type{};
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
        virtual void BindBindingSet(const PipelineState& pipeline, BindingSet* bindingSet) = 0;
        virtual void DrawIndexedIndirect(const Buffer& buffer, usize offset, u32 drawCount, u32 stride) = 0;
        virtual void BindPipelineState(const PipelineState& pipeline) = 0;
        virtual void Dispatch(u32 x, u32 y, u32 z) = 0;
        virtual void TraceRays(PipelineState pipeline, u32 x, u32 y, u32 z) = 0;
        virtual void SetScissor(const Rect& rect) = 0;
        virtual void BeginLabel(const StringView& name, const Vec4& color) = 0;
        virtual void EndLabel() = 0;
        virtual void ResourceBarrier(const ResourceBarrierInfo& resourceBarrierInfo) = 0;
        virtual void CopyBuffer(Buffer srcBuffer, Buffer dstBuffer, const Span<BufferCopyInfo>& info) = 0;
        virtual void CopyBufferToTexture(Buffer srcBuffer, Texture texture, const Span<BufferImageCopy>& regions) = 0;
        virtual void SubmitAndWait(GPUQueue queue) = 0;

    };

    struct DeviceFeatures
    {
        bool raytraceSupported{};
        bool bindlessSupported{};
        bool multiDrawIndirectSupported{};
    };


    inline u32 GetFormatSize(Format format)
    {
        switch (format)
        {
        case Format::R: return 8;
        case Format::R16F: return 16;
        case Format::R32F: return 32;
        case Format::RG: return 8 * 2;
        case Format::RG16F: return 16 * 2;
        case Format::RG32F: return 32 * 2;
        case Format::RGB: return 8 * 3;
        case Format::RGB16F: return 16 * 3;
        case Format::RGB32F: return 32 * 3;
        case Format::RGBA: return 8 * 4;
        case Format::RGBA16F: return 16 * 4;
        case Format::RGBA32F: return 32 * 4;
        case Format::BGRA: return 8 * 4;
        case Format::Depth:
        case Format::Undefined:
            break;
        }
        return 0;
    }

}
