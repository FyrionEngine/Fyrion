#pragma once

#include "GraphicsTypes.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Optional.hpp"
#include "Fyrion/Core/SharedPtr.hpp"


namespace Fyrion
{
    class RenderGraph;
    class RenderGraphPass;

    class FY_API RenderGraphAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        Array<String>          GetPasses() const;
        Array<RenderGraphEdge> GetEdges() const;

        StringView GetColorOutput() const;
        StringView GetDepthOutput() const;


        static void RegisterType(NativeTypeHandler<RenderGraphAsset>& type);

    private:
        Array<String>          passes;
        Array<RenderGraphEdge> edges;

        String colorOutput;
        String depthOutput;
    };

    struct FY_API RenderGraphResource
    {
        String                      fullName{};
        RenderGraphResourceCreation creation{};
        TextureCreation             textureCreation{};
        Texture                     texture{};
        Buffer                      buffer{};
        VoidPtr                     reference{};
        ResourceLayout              currentLayout{};

        ~RenderGraphResource();
    };

    struct FY_API RenderGraphInput
    {
        String                         fullName{};
        RenderGraphResourceCreation    inputCreation{};
        SharedPtr<RenderGraphResource> resource{};
    };

    class FY_API RenderGraphNode
    {
    public:
        RenderGraphNode(const StringView name, const RenderGraphPassCreation& creation) : name(name), creation(creation) {}

        friend class RenderGraph;

        ~RenderGraphNode();

        RenderPass GetRenderPass() const;

        Texture GetInputTexture(StringView view) const;
        Texture GetOutputTexture(StringView view) const;

        RenderGraphResource* GetInputResource(StringView view) const;
        RenderGraphResource* GetOutputResource(StringView view) const;

    private:
        String                                          name{};
        RenderGraphPassCreation                         creation{};
        RenderPass                                      renderPass{};
        RenderGraphPass*                                renderGraphPass = nullptr;
        TypeHandler*                                    renderGraphPassTypeHandler = nullptr;
        HashMap<String, SharedPtr<RenderGraphInput>>    inputs{};
        HashMap<String, SharedPtr<RenderGraphResource>> outputs{};
        Extent3D                                        extent{};
        Optional<Vec4>                                  clearColor{};
        Optional<ClearDepthStencilValue>                clearDepthStencil{};

        void CreateRenderPass();
    };

    class FY_API RenderGraphPass
    {
    public:
        RenderGraphNode* node = nullptr;
        RenderGraph*     graph = nullptr;

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
        RenderGraph(RenderGraphAsset* asset);
        RenderGraph(Extent extent, RenderGraphAsset* asset);
        ~RenderGraph();

        Extent GetViewportExtent() const;

        void Resize(Extent p_extent);
        void SetCameraData(const CameraData& cameraData);

        const CameraData& GetCameraData() const;

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
        SharedPtr<RenderGraphResource>    colorOutput;
        SharedPtr<RenderGraphResource>    depthOutput;
        CameraData                        cameraData;

        void                           Create();
        SharedPtr<RenderGraphResource> CreateResource(const StringView& fullName, const RenderGraphResourceCreation& creation);

        void RecordCommands(RenderCommands& cmd, f64 deltaTime);
        void BlitSwapchapin(RenderCommands& cmd);
    };

    template <typename T = void>
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

        RenderGraphPassBuilder& Shader(StringView shaderPath)
        {
            FY_ASSERT(false, "not implemented yet");
            return *this;
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
