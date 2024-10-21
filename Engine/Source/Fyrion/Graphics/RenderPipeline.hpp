#pragma once

namespace Fyrion
{
    class RenderGraph;

    struct RenderPipeline
    {
        virtual ~RenderPipeline() = default;
        virtual void BuildRenderGraph(RenderGraph& rg) = 0;
    };
}
