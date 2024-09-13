#include "RenderStorage.hpp"

#include <optional>

#include "Graphics.hpp"
#include "GraphicsTypes.hpp"
#include "Assets/MaterialAsset.hpp"
#include "Fyrion/Engine.hpp"
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
        u64 textureIndex = 0;
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
        BindingVar* textures;
        u64         textureCount = 0;

        void UploadTextureData()
        {
            for (TextureAsset* textureAsset : pendingLoadingTextures)
            {
                if (TextureAssetHandler* handler = dynamic_cast<TextureAssetHandler*>(textureAsset->handler))
                {
                    textures->SetTextureAt(textureAsset->CreateTexture(), handler->textureIndex);
                }
            }
            pendingLoadingTextures.Clear();
        }

        void RequestTextureLoad(TextureAsset* textureAsset)
        {
            if (textureAsset)
            {
                if (!textureAsset->handler)
                {
                    TextureAssetHandler* textureAssetHandler = MemoryGlobals::GetDefaultAllocator().Alloc<TextureAssetHandler>();
                    textureAssetHandler->textureIndex = textureCount++;
                    textureAsset->handler = textureAssetHandler;
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

        // for (MaterialAsset* material : materials)
        // {
        //     RequestMaterialLoad(material);
        // }
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

    BindingSet* RenderStorage::GetBindlessTextures()
    {
        return bindlessTextures;
    }


    void RenderStorageDestroy()
    {
        // if (bindlessTextures)
        // {
        //     Graphics::DestroyBindingSet(bindlessTextures);
        //     bindlessTextures = nullptr;
        // }
    }

    void RenderStorage::Init()
    {
        // DescriptorLayout descriptorLayout{
        //     .set = 4,
        //     .bindings = {
        //         DescriptorBinding
        //         {
        //             .binding = 0,
        //             .name = "textures",
        //             .descriptorType = DescriptorType::SampledImage,
        //             .renderType = RenderType::RuntimeArray,
        //             .shaderStage = ShaderStage::Pixel
        //         }
        //     }
        // };
        // bindlessTextures = Graphics::CreateBindingSet({descriptorLayout});
        // textures = bindlessTextures->GetVar("textures");
        //
        // Event::Bind<OnShutdown, RenderStorageDestroy>();
    }


    void RenderStorage::UpdateResources()
    {
        UploadTextureData();
        UploadMaterialData();
    }
}
