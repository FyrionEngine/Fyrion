#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Core/Math.hpp"


namespace Fyrion
{

    struct RenderGraphPassAsset
    {
        constexpr static u32 Pass = 0;
    };

    struct RenderGraphEdgeAsset
    {
        constexpr static u32 Origin = 0;
        constexpr static u32 Dest = 1;
    };

    struct RenderGraphAsset
    {
        constexpr static u32 Passes = 0;
        constexpr static u32 Edges = 1;
        constexpr static u32 ColorOutput = 2;
        constexpr static u32 DepthOutput = 3;
    };


    class FY_API RenderGraph
    {
    public:
        Extent  GetViewportExtent();
        void    Resize(const Extent& extent);
        Texture GetColorOutput() const;
        Texture GetDepthOutput() const;

        static RenderGraph* Create(const Extent& extent, RID renderGraphId);
    private:

    };
}
