#include "RenderGraph.hpp"

namespace Fyrion
{
    RenderPassBuilder::RenderPassBuilder(RenderGraph* renderGraph) : renderGraph(renderGraph)
    {

    }

    RenderPassBuilder& RenderPassBuilder::Read(const RenderGraphResource* resource)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Read(StringView name, const RenderGraphResource* resource)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Write(const RenderGraphResource* resource)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Init(const std::function<void(RenderGraphNode& node)>& func)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Update(const std::function<void(RenderGraphNode& node)>& func)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Render(const std::function<void(RenderGraphNode& node, RenderCommands& cmd)>& func)
    {
        return *this;
    }

    RenderPassBuilder RenderGraph::AddPass(StringView name, RenderGraphPassType type)
    {
        return {this};
    }

    RenderGraphResource* RenderGraph::Create(const RenderGraphResourceCreation& creation)
    {
        return {};
    }
}
