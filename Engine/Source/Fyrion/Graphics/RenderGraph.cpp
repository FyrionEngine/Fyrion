#include "RenderGraph.hpp"


namespace Fyrion
{
    Extent RenderGraph::GetViewportExtent()
    {
        return {};
    }

    void RenderGraph::Resize(const Extent& extent)
    {

    }

    Texture RenderGraph::GetColorOutput() const
    {
        return {};
    }

    Texture RenderGraph::GetDepthOutput() const
    {
        return {};
    }

    RenderGraph* RenderGraph::Create(const Extent& extent, RID renderGraphId)
    {
        return nullptr;
    }
}
