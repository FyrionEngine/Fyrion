#pragma once
#include "GraphicsTypes.hpp"

namespace Fyrion::RenderUtils
{
    FY_API AABB    CalculateMeshAABB(const Array<VertexStride>& vertices);
    FY_API void    CalcTangents(Array<VertexStride>& vertices, const Array<u32>& indices, bool useMikktspace = true);
    FY_API Texture ConvertEquirectangularToCubemap(RenderCommands& cmd, GPUQueue queue, Texture originTexture, Format format, Extent cubemapSize);
}
