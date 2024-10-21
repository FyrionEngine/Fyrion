#pragma once

#include <functional>

#include "GraphicsTypes.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Optional.hpp"
#include "Fyrion/Core/SharedPtr.hpp"

namespace Fyrion
{
    class RenderGraph;

    struct RenderGraphResource
    {

    };

    class FY_API RenderGraphNode
    {
    public:
        RenderPass GetRenderPass()
        {
            return renderPass;
        }

        BindingSet* GetBindingSet()
        {
            return nullptr;
        }

    private:
        RenderPass renderPass{};
    };

    class FY_API RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderGraph* renderGraph);

        FY_NO_COPY_CONSTRUCTOR(RenderPassBuilder);

        RenderPassBuilder& Read(const RenderGraphResource* resource);
        RenderPassBuilder& Read(StringView name, const RenderGraphResource* resource);
        RenderPassBuilder& Write(const RenderGraphResource* resource);
        RenderPassBuilder& Init(const std::function<void(RenderGraphNode& node)>& func);
        RenderPassBuilder& Update(const std::function<void(RenderGraphNode& node)>& func);
        RenderPassBuilder& Render(const std::function<void(RenderGraphNode& node, RenderCommands& cmd)>& func);

    private:
        RenderGraph* renderGraph;
    };

    class FY_API RenderGraph
    {
    public:
        RenderPassBuilder    AddPass(StringView name, RenderGraphPassType type);
        RenderGraphResource* Create(const RenderGraphResourceCreation& creation);

    private:
    };
}
