#pragma once

#include "GraphicsTypes.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/SharedPtr.hpp"


namespace Fyrion
{
    struct RenderGraphPass;

    struct RenderGraphEdge
    {
        String origin{};
        String dest{};
    };

    struct RenderGraphAsset
    {
        constexpr static u32 Passes = 0;
        constexpr static u32 Edges = 1;
        constexpr static u32 ColorOutput = 2;
        constexpr static u32 DepthOutput = 3;
    };

    class RenderGraph;

    enum class RenderGraphResourceType
    {
        Buffer      = 0,
        Texture     = 1,
        Attachment  = 2,
        Reference   = 3,
    };

    enum class RenderGraphPassType
    {
        Other           = 0,
        Graphics        = 1,
        Compute         = 2
    };

    struct RenderGraphResourceCreation
    {
        String                  name{};
        RenderGraphResourceType type{};
        Extent3D                size{};
        Vec2                    scale{};
        LoadOp                  loadOp{LoadOp::Clear};
        Vec4                    cleanValue{};
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

    struct RenderGraphResource
    {
        RenderGraphResourceCreation creation{};
        Texture                     texture{};
        Extent3D                    extent{};
        Buffer                      buffer{};
        VoidPtr                     reference{};
        bool                        destroyed{};
        bool                        ownResource{};
    };

    struct InputNodeResource
    {
        RenderGraphResourceCreation    creation{};
        SharedPtr<RenderGraphResource> resource{};
    };


    class FY_API RenderGraphNode
    {
    public:
        Texture    GetNodeInputTexture(const StringView& inputName);
        RenderPass GetRenderPass();
        Extent3D   GetRenderPassExtent();
        Buffer     GetOutputBuffer(const StringView& outputName);
        Buffer     GetInputBuffer(const StringView& outputName);
        void       SetOutputBuffer(const StringView& outputName, const Buffer& buffer);
        void       SetOutputReference(const StringView& outputName, VoidPtr reference);
        VoidPtr    GetInputReference(const StringView& inputName);
        explicit   RenderGraphNode(const String& name);


        template <typename Type>
        Type& GetInputReference(const StringView& inputName)
        {
            return *static_cast<Type*>(GetInputReference(inputName));
        }

    private:
        String                                          m_name{};
        RenderGraphPassCreation                         m_creation{};
        RenderPass                                      m_renderPass{};
        RenderGraphPass*                                m_renderGraphPass{};
        TypeID                                          m_renderGraphPassId{};
        HashMap<String, InputNodeResource>              m_inputs{};
        HashMap<String, SharedPtr<RenderGraphResource>> m_outputs{};
        TypeID                                          m_renderPassTypeId{};
        Extent3D                                        m_extent{};
        Array<Vec4>                                     m_clearValues{};
        Vec3                                            m_debugColor{};

        void CreateRenderPass();

        friend class RenderGraph;
    };

    struct RenderGraphPass
    {
        RenderGraphNode* node = nullptr;

        virtual void Init() {}
        virtual void Update(f64 deltaTime) {}
        virtual void Render(const RenderCommands& cmd) {}
        virtual void Destroy() {}
        virtual void Resize(Extent newExtent) {}
        virtual ~RenderGraphPass() = default;
    };

    using RenderGraphResMap = HashMap<String, SharedPtr<RenderGraphResource>>;


    class FY_API RenderGraph
    {
    public:
        RenderGraph(u32 id, const Extent& viewportSize);

        Extent  GetViewportExtent();
        void    Resize(Extent viewportSize);
        void    RecordCommands(RenderCommands& renderCommands, f64 deltaTime);
        Texture GetColorOutput() const;
        Texture GetDepthOutput() const;
        void    Destroy();

        static RenderGraph* Create(const Extent& viewportSize, RID renderGraphId);
        static void         RegisterPass(const RenderGraphPassCreation& renderGraphPassCreation);
    private:
        u32                               m_id{};
        Extent                            m_viewportSize{};
        Array<SharedPtr<RenderGraphNode>> m_nodes{};
        RenderGraphResMap                 m_resources{};

        void        CreateResource(const RenderGraphResourceCreation& renderGraphResourceCreation, RenderGraphResource& renderGraphResource) const;
        static void DestroyResource(RenderGraphResource& renderGraphResource);
    };
}
