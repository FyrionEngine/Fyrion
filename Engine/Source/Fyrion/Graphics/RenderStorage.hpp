#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion::RenderStorage
{
    FY_API void                 AddOrUpdateMeshToRender(usize address, const Mat4& model, MeshAsset* mesh, Span<MaterialAsset*> materials);
    FY_API void                 RemoveMeshFromRender(usize address);
    FY_API Span<MeshRenderData> GetMeshesToRender();
}
