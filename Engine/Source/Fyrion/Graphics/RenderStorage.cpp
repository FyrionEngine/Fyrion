#include "RenderStorage.hpp"
#include "GraphicsTypes.hpp"
#include "Fyrion/Core/HashMap.hpp"

namespace Fyrion
{
    namespace
    {
        HashMap<usize, usize> meshRenderDataIndices{};
        Array<MeshRenderData> meshRenderDataArray{};
    }

    void RenderStorage::AddOrUpdateMeshToRender(usize address, const Mat4& model, MeshAsset* mesh)
    {
        auto it = meshRenderDataIndices.Find(address);
        if (it == meshRenderDataIndices.end())
        {
            it = meshRenderDataIndices.Emplace(address, meshRenderDataArray.Size()).first;
            meshRenderDataArray.EmplaceBack();
        }

        MeshRenderData& data = meshRenderDataArray[it->second];
        data.address = address;
        data.model = model,
        data.mesh = mesh;
    }

    Span<MeshRenderData> RenderStorage::GetMeshesToRender()
    {
        return meshRenderDataArray;
    }

    void RenderStorage::RemoveMeshFromRender(usize address)
    {
        if (auto it = meshRenderDataIndices.Find(address))
        {
            MeshRenderData& last = meshRenderDataArray.Back();
            meshRenderDataIndices[last.address] = it->second;
            meshRenderDataArray[it->second] = Traits::Move(last);

            meshRenderDataArray.PopBack();
            meshRenderDataIndices.Erase(it);
        }

    }
}
