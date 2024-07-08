#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"


namespace Fyrion
{
    class RenderGraphPass;

    class FY_API RenderGraphAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        StringView GetDisplayName() const override
        {
            return "Render Graph";
        }

        static void RegisterType(NativeTypeHandler<RenderGraphAsset>& type);

        Array<String>          GetPasses() const;
        Array<RenderGraphEdge> GetEdges() const;

    private:
        Array<String>          passes;
        Array<RenderGraphEdge> edges;
    };

    struct FY_API RenderGraphResource
    {
        String                      fullName{};
        RenderGraphResourceCreation creation{};
        TextureCreation             textureCreation{};
        Texture                     texture{};
        Buffer                      buffer{};
        VoidPtr                     reference{};

        ~RenderGraphResource();
    };

    class FY_API RenderGraphNode
    {
    public:
        RenderGraphNode(const StringView name, const RenderGraphPassCreation& creation) : name(name), creation(creation) {}

        friend class RenderGraph;

        ~RenderGraphNode();

    private:
        String                                          name{};
        RenderGraphPassCreation                         creation{};
        RenderPass                                      renderPass{};
        RenderGraphPass*                                renderGraphPass = nullptr;
        TypeHandler*                                    renderGraphPassTypeHandler = nullptr;
        HashMap<String, SharedPtr<RenderGraphResource>> inputs{};
        HashMap<String, SharedPtr<RenderGraphResource>> outputs{};
        Extent3D                                        extent{};
        Array<Vec4>                                     clearValues{};

        void CreateRenderPass();
    };

    class FY_API RenderGraphPass
    {
    public:
        RenderGraphNode* node = nullptr;

        virtual void Init() {}
        virtual void Update(f64 deltaTime) {}
        virtual void Render(f64 deltaTime, RenderCommands& cmd) {}
        virtual void Destroy() {}
        virtual void Resize(Extent3D newExtent) {}
        virtual      ~RenderGraphPass() = default;
    };


    using RenderGraphResMap = HashMap<String, SharedPtr<RenderGraphResource>>;

    class FY_API RenderGraph
    {
    public:
        RenderGraph(Extent extent, RenderGraphAsset* asset);

        Extent GetViewportExtent() const;

        void Resize(Extent p_extent);

        Texture GetColorOutput() const;
        Texture GetDepthOutput() const;

        static void RegisterType(NativeTypeHandler<RenderGraph>& type);
        static void RegisterPass(const RenderGraphPassCreation& renderGraphPassCreation);
        static void SetRegisterSwapchainRenderEvent(bool p_registerSwapchainRenderEvent);

    private:
        RenderGraphAsset*                 asset = nullptr;
        Extent                            viewportExtent;
        Array<SharedPtr<RenderGraphNode>> nodes;
        RenderGraphResMap                 resources;


        void                           Create();
        SharedPtr<RenderGraphResource> CreateResource(const StringView& fullName, const RenderGraphResourceCreation& creation);
    };

    template <typename T>
    class RenderGraphPassBuilder
    {
    public:
        static RenderGraphPassBuilder Builder(RenderGraphPassType type)
        {
            RenderGraphPassBuilder builder;
            builder.creation.name = GetTypeName<T>();
            builder.creation.type = type;
            return builder;
        }

        RenderGraphPassBuilder& Input(const RenderGraphResourceCreation& resource)
        {
            creation.inputs.EmplaceBack(resource);
            return *this;
        }

        RenderGraphPassBuilder& Output(const RenderGraphResourceCreation& resource)
        {
            creation.outputs.EmplaceBack(resource);
            return *this;
        }

        ~RenderGraphPassBuilder()
        {
            RenderGraph::RegisterPass(creation);
        }

    private:
        RenderGraphPassCreation creation{};
    };
}
