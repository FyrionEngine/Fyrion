#include "DefaultRenderPipelineNodes.hpp"

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"


namespace Fyrion
{

    void DefaultRenderPipelineNodesInit()
    {
        auto geometryRender = Registry::Type<GeometryRender>("Fyrion::GeometryRender");
        geometryRender.Field<&GeometryRender::vertexBuffer>("vertexBuffer");
        geometryRender.Field<&GeometryRender::indexBuffer>("indexBuffer");
        geometryRender.Attribute<ResourceGraphOutput>("Output/Geometry Render");
    }
}
