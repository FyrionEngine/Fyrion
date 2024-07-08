#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Asset/Asset.hpp"


namespace Fyrion
{
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


    class FY_API RenderGraphNode
    {
    public:
    };

    class FY_API RenderGraphPass
    {
    public:
        RenderGraphNode* node = nullptr;

        virtual void Init() {}
        virtual void Update(f64 deltaTime) {}
        virtual void Render(f64 deltaTime, const RenderCommands& cmd) {}
        virtual void Destroy() {}
        virtual void Resize(Extent newExtent) {}
        virtual      ~RenderGraphPass() = default;
    };


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
        RenderGraphAsset* asset = nullptr;
        Extent            extent;
    };

    template <typename T>
    class RenderGraphPassBuilder
    {
    public:

        static RenderGraphPassBuilder Builder()
        {
            RenderGraphPassBuilder builder;
            builder.creation.name = GetTypeName<T>();
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
