#pragma once
#include "GraphicsTypes.hpp"

namespace Fyrion::MeshUtils
{
    AABB CalculateMeshAABB(const Array<VertexStride>& vertices);
    void CalcTangents(Array<VertexStride>& vertices, const Array<u32>& indices, bool useMikktspace = true);
}
