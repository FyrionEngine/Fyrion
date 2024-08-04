#include "RenderStorage.hpp"

#include <optional>

#include "GraphicsTypes.hpp"
#include "Fyrion/Core/HashMap.hpp"

namespace Fyrion
{
    namespace
    {
        HashMap<usize, usize> meshRenderDataIndices{};
        Array<MeshRenderData> meshRenderDataArray{};

        TextureAsset* skyboxAsset = nullptr;

        std::optional<DirectionalLight> directionalLight;
    }


    void RenderStorage::AddOrUpdateMeshToRender(usize address, const Mat4& model, MeshAsset* mesh, Span<MaterialAsset*> materials)
    {
        auto it = meshRenderDataIndices.Find(address);
        if (it == meshRenderDataIndices.end())
        {
            it = meshRenderDataIndices.Emplace(address, meshRenderDataArray.Size()).first;
            meshRenderDataArray.EmplaceBack();
        }

        MeshRenderData& data = meshRenderDataArray[it->second];
        data.address = address;
        data.model = model;
        data.mesh = mesh;
        data.materials = materials;
    }

    Span<MeshRenderData> RenderStorage::GetMeshesToRender()
    {
        return meshRenderDataArray;
    }

    TextureAsset* RenderStorage::GetSkybox()
    {
        return skyboxAsset;
    }

    void RenderStorage::AddSkybox(TextureAsset* skybox)
    {
        skyboxAsset = skybox;
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

    void RenderStorage::AddDirectionalLight(usize address, const DirectionalLight& dirLight)
    {
        directionalLight = dirLight;
    }

    void RenderStorage::RemoveDirectionalLight(usize address)
    {
        directionalLight.reset();
    }

    DirectionalLight* RenderStorage::GetDirectionalLight()
    {
        if (directionalLight)
        {
            return &directionalLight.value();
        }
        return nullptr;
    }
}
