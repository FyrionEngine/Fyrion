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

    struct RenderGraphResource
    {
        RenderGraphResourceCreation creation;
        TextureCreation             textureCreation{};
        ResourceLayout              currentLayout;

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

        u32                         id{};
        String                      name{};
        RenderGraphPassType         type{};
        RenderPass                  renderPass{};
        Array<Resource>             inputs;
        Array<RenderGraphResource*> outputs;
        Optional<Vec4>              clearValue;
        bool                        clearDepth{};

        void CreateRenderPass();
    };

    class FY_API RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderGraphPass* pass);

        FY_NO_COPY_CONSTRUCTOR(RenderPassBuilder);

        RenderPassBuilder& Read(RenderGraphResource* resource);
        RenderPassBuilder& Write(RenderGraphResource* resource);
        RenderPassBuilder& ClearColor(const Vec4& color);
        RenderPassBuilder& ClearDepth(bool clear);

        RenderPassBuilder& Init(const std::function<void(RenderGraphPass& pass)>& func);
        RenderPassBuilder& Update(const std::function<void(RenderGraphPass& pass)>& func);
        RenderPassBuilder& Render(const std::function<void(RenderGraphPass& pass, RenderCommands& cmd)>& func);

    private:
        RenderGraphPass* pass;
    };

    class FY_API RenderGraph
    {
    public:
        RenderPassBuilder    AddPass(StringView name, RenderGraphPassType type);
        RenderGraphResource* Create(const RenderGraphResourceCreation& creation);

        void Bake(Extent viewportExtent);

    private:
        Extent                                viewportExtent;
        Array<SharedPtr<RenderGraphResource>> resources;
        Array<SharedPtr<RenderGraphPass>>     passes;

        void RecordCommands(RenderCommands& cmd, f64 deltaTime);

        void CreateResources();
    };
}
