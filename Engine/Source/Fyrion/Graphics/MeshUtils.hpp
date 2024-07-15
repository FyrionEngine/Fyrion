#pragma once
#include "GraphicsTypes.hpp"

namespace Fyrion::MeshUtils
{
    AABB CalculateMeshAABB(const Array<VertexData>& vertices);
    void CalcTangents(Array<VertexData>& vertices, const Array<u32>& indices, bool useMikktspace = true);
}
