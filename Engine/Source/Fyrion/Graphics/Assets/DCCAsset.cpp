#include "DCCAsset.hpp"

namespace Fyrion
{
    MaterialAsset* DCCAsset::FindMaterialByName(const StringView& materialName)
    {
        for (Asset* asset : GetChildrenAssets())
        {
            if (MaterialAsset* materialAsset = dynamic_cast<MaterialAsset*>(asset))
            {
                if (materialAsset->GetName() == materialName)
                {
                    return materialAsset;
                }
            }
        }
        return nullptr;
    }

    MeshAsset* DCCAsset::FindMeshByName(const StringView& meshName)
    {
        for (Asset* asset : GetChildrenAssets())
        {
            if (MeshAsset* meshAsset = dynamic_cast<MeshAsset*>(asset))
            {
                if (meshAsset->GetName() == meshName)
                {
                    return meshAsset;
                }
            }
        }
        return nullptr;
    }

    TextureAsset* DCCAsset::FindTextureByName(const StringView& textureName)
    {
        for (Asset* asset : GetChildrenAssets())
        {
            if (TextureAsset* textureAsset = dynamic_cast<TextureAsset*>(asset))
            {
                if (textureAsset->GetName() == textureName)
                {
                    return textureAsset;
                }
            }
        }
        return nullptr;
    }

    SceneObjectAsset* DCCAsset::GetSceneObjectAsset()
    {
        for (Asset* asset : GetChildrenAssets())
        {
            if (SceneObjectAsset* sceneObjectAsset = dynamic_cast<SceneObjectAsset*>(asset))
            {
                return sceneObjectAsset;
            }
        }
        return nullptr;
    }


    void DCCAsset::RegisterType(NativeTypeHandler<DCCAsset>& type)
    {
        type.Field<&DCCAsset::scaleFactor>("scaleFactor");
        type.Field<&DCCAsset::mergeMaterials>("mergeMaterials");
        type.Field<&DCCAsset::mergeMeshes>("mergeMeshes");
    }
}
