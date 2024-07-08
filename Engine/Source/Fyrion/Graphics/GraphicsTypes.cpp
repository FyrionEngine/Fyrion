#include "GraphicsTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RenderGraphEdge::RegisterType(NativeTypeHandler<RenderGraphEdge>& type)
    {
        type.Field<&RenderGraphEdge::origin>("origin");
        type.Field<&RenderGraphEdge::nodeOrigin>("nodeOrigin");
        type.Field<&RenderGraphEdge::dest>("dest");
        type.Field<&RenderGraphEdge::nodeDest>("nodeDest");
    }
}
