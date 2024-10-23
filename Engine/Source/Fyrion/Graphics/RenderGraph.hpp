#pragma once

#include <functional>

#include "GraphicsTypes.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Optional.hpp"
#include "Fyrion/Core/SharedPtr.hpp"

namespace Fyrion
{
    class RenderGraph;
    class RenderGraphPass;

    struct FY_API RenderGraphResource
    {
        RenderGraphResourceCreation creation;
        TextureCreation             textureCreation{};
        ResourceLayout              currentLayout = ResourceLayout::Undefined;

        union
        {
            Texture texture = {};
            Buffer  buffer;
            VoidPtr reference;
        };

        struct ResourceEdges
        {
            RenderGraphPass*        writePass;
            Array<RenderGraphPass*> readPass;
        };

        Array<ResourceEdges> edges;

        void WriteIn(RenderGraphPass* pass);
        void ReadIn(RenderGraphPass* pass);

        ~RenderGraphResource();
    };


    struct FY_API RenderGraphPassHandler
    {
        virtual ~RenderGraphPassHandler() = default;

        RenderGraphPass* pass = nullptr;
        RenderGraph* rg = nullptr;

        virtual void Init() {}
        virtual void Update(f64 deltaTime) {}
        virtual void Resize(Extent3D extent) {}
        virtual void Render(RenderCommands& cmd) {}
        virtual void Destroy() {}
    };

    class FY_API RenderGraphPass
    {
    public:
        RenderPass GetRenderPass() const;
        StringView GetName() const;

        friend class RenderGraph;
        friend class RenderPassBuilder;

        ~RenderGraphPass();

    private:
        struct Resource
        {
            RenderGraphResource* resource;
            String               name;
        };

        u32                     id{};
        Extent3D                extent{};
        String                  name{};
        RenderGraphPassType     type{};
        RenderPass              renderPass{};
        Array<Resource>         inputs;
        Array<Resource>         outputs;
        Optional<Vec4>          clearValue;
        bool                    clearDepth{};
        RenderGraphPassHandler* handler = nullptr;

        void CreateRenderPass();
    };

    class FY_API RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderGraph* rg, RenderGraphPass* pass);

        FY_NO_COPY_CONSTRUCTOR(RenderPassBuilder);

        RenderPassBuilder& Read(RenderGraphResource* resource);
        RenderPassBuilder& Write(RenderGraphResource* resource);
        RenderPassBuilder& ClearColor(const Vec4& color);
        RenderPassBuilder& ClearDepth(bool clear);

        RenderPassBuilder& Handler(RenderGraphPassHandler* handler);

    private:
        RenderGraph* rg;
        RenderGraphPass* pass;
    };

    class FY_API RenderGraph
    {
    public:
        RenderPassBuilder    AddPass(StringView name, RenderGraphPassType type);
        RenderGraphResource* Create(const RenderGraphResourceCreation& creation);
        void                 Resize(Extent extent);
        void                 Bake(Extent extent);
        Extent               GetViewportExtent() const;
        void                 SetCameraData(const CameraData& cameraData);
        const CameraData&    GetCameraData() const;
        void                 ColorOutput(RenderGraphResource* resource);
        void                 DepthOutput(RenderGraphResource* resource);
        Texture              GetColorOutput() const;
        Texture              GetDepthOutput() const;

    private:
        Extent                                viewportExtent;
        Array<SharedPtr<RenderGraphResource>> resources;
        Array<SharedPtr<RenderGraphPass>>     passes;
        CameraData                            cameraData;
        RenderGraphResource*                  colorOutput = {};
        RenderGraphResource*                  depthOutput = {};

        void RecordCommands(RenderCommands& cmd, f64 deltaTime);

        void CreateResources();
    };
}
