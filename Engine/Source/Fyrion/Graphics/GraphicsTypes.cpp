#include "GraphicsTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RenderGraphEdge::RegisterType(NativeTypeHandler<RenderGraphEdge>& type)
    {
        type.Field<&RenderGraphEdge::output>("output");
        type.Field<&RenderGraphEdge::nodeOutput>("nodeOutput");
        type.Field<&RenderGraphEdge::input>("input");
        type.Field<&RenderGraphEdge::nodeInput>("nodeInput");
    }
}
