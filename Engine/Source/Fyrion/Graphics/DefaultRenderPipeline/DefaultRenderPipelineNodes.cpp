#include "DefaultRenderPipelineNodes.hpp"

#include "Fyrion/Core/GraphTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"


namespace Fyrion
{

    void DefaultRenderPipelineNodesInit()
    {
        auto geometryRender = Registry::Type<GeometryRender>("Fyrion::GeometryRender");
        geometryRender.Field<&GeometryRender::vertexBuffer>("vertexBuffer").Attribute<GraphInput>();
        geometryRender.Field<&GeometryRender::indexBuffer>("indexBuffer").Attribute<GraphInput>();
        geometryRender.Attribute<ResourceGraphOutput>("Outputs/Geometry Render");
    }
}
