#include "RenderStorage.hpp"

#include <optional>

#include "Graphics.hpp"
#include "GraphicsTypes.hpp"
#include "Assets/MaterialAsset.hpp"
#include "Fyrion/Core/HashMap.hpp"

namespace Fyrion
{
    struct MaterialAssetHandler : RenderAssetHandler
    {
        u64 refCount = 0;

        void RefIncrease() override
        {
            refCount++;
        }

        void RefDecrese() override
        {
            refCount--;
        }
    };

    struct TextureAssetHandler : RenderAssetHandler
    {
        u64 refCount = 0;

        void RefIncrease() override
        {
            refCount++;
        }

        void RefDecrese() override
        {
            refCount--;
        }
    };


    namespace
    {
        HashMap<usize, usize> meshRenderDataIndices{};
        Array<MeshRenderData> meshRenderDataArray{};

        TextureAsset* skyboxAsset = nullptr;

        std::optional<DirectionalLight> directionalLight;

        Array<TextureAsset*>  pendingLoadingTextures;
        Array<MaterialAsset*> pendingLoadingMaterials;

        BindingSet* bindlessTextures;

        void UploadTextureData()
        {
            for (TextureAsset* textureAsset : pendingLoadingTextures)
            {
                Texture texture = textureAsset->CreateTexture();
            }
            pendingLoadingTextures.Clear();
        }

        void RequestTextureLoad(TextureAsset* textureAsset)
        {
            if (textureAsset)
            {
                if (!textureAsset->handler)
                {
                    textureAsset->handler = MemoryGlobals::GetDefaultAllocator().Alloc<TextureAssetHandler>();
                    pendingLoadingTextures.EmplaceBack(textureAsset);
                }
                textureAsset->handler->RefIncrease();
            }
        }

        void UploadMaterialData() {}

        void RequestMaterialLoad(MaterialAsset* material)
        {
            if (material)
            {
                if (!material->handler)
                {
                    material->handler = MemoryGlobals::GetDefaultAllocator().Alloc<MaterialAssetHandler>();
                    RequestTextureLoad(material->GetBaseColorTexture());
                    RequestTextureLoad(material->GetNormalTexture());
                    RequestTextureLoad(material->GetMetallicTexture());
                    RequestTextureLoad(material->GetMetallicRoughnessTexture());
                    RequestTextureLoad(material->GetAoTexture());
                    RequestTextureLoad(material->GetEmissiveTexture());
                    pendingLoadingMaterials.EmplaceBack(material);
                }
                material->handler->RefIncrease();
            }
        }
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

        for (MaterialAsset* material : materials)
        {
            RequestMaterialLoad(material);
        }
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

    void RenderStorage::Init()
    {
        DescriptorLayout descriptorLayout{
            .set = 1,
            .bindings = {
                DescriptorBinding
                {
                    .binding = 0,
                    .descriptorType = DescriptorType::SampledImage,
                    .renderType = RenderType::RuntimeArray
                }
            }
        };
        bindlessTextures = Graphics::CreateBindingSet({descriptorLayout});
    }

    void RenderStorage::UpdateResources()
    {
        UploadTextureData();
        UploadMaterialData();
    }
}
