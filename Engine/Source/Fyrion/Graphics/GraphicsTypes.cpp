#include "GraphicsTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void MeshPrimitive::RegisterType(NativeTypeHandler<MeshPrimitive>& type)
    {
        type.Field<&MeshPrimitive::firstIndex>("firstIndex");
        type.Field<&MeshPrimitive::indexCount>("indexCount");
        type.Field<&MeshPrimitive::materialIndex>("materialIndex");
    }

    void RenderGraphEdge::RegisterType(NativeTypeHandler<RenderGraphEdge>& type)
    {
        type.Field<&RenderGraphEdge::output>("output");
        type.Field<&RenderGraphEdge::nodeOutput>("nodeOutput");
        type.Field<&RenderGraphEdge::input>("input");
        type.Field<&RenderGraphEdge::nodeInput>("nodeInput");
    }
}
