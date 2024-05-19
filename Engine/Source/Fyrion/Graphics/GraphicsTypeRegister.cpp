#include "Fyrion/Core/Registry.hpp"
#include "GraphicsTypes.hpp"
#include "RenderGraph.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    void RegisterGraphicsTypes()
    {
        Registry::Type<ShaderStageInfo>();
        Registry::Type<Array<ShaderStageInfo>>();
        Registry::Type<ShaderInfo>();
        Registry::Type<Buffer>();


        Registry::Type<BufferUsage>();


        ResourceTypeBuilder<RenderGraphPassAsset>::Builder()
            .Value<RenderGraphPassAsset::Pass, String>("Pass")
            .Build();

        ResourceTypeBuilder<RenderGraphEdgeAsset>::Builder()
            .Value<RenderGraphEdgeAsset::Dest, String>("Dest")
            .Value<RenderGraphEdgeAsset::Origin, String>("Origin")
            .Build();

        ResourceTypeBuilder<RenderGraphAsset>::Builder("RenderGraph")
            .SubObjectSet<RenderGraphAsset::Passes>("Passes")
            .SubObjectSet<RenderGraphAsset::Edges>("Edges")
            .Value<RenderGraphAsset::ColorOutput, String>("ColorOutput")
            .Value<RenderGraphAsset::DepthOutput, String>("DepthOutput")
            .Build();

    }
}
