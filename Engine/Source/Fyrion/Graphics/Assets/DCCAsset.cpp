#include "DCCAsset.hpp"

namespace Fyrion
{
    MaterialAsset* DCCAsset::FindMaterialByName(const StringView& materialName)
    {
        // for (Asset* asset : GetChildren())
        // {
        //     if (MaterialAsset* materialAsset = dynamic_cast<MaterialAsset*>(asset))
        //     {
        //         if (materialAsset->GetName() == materialName)
        //         {
        //             return materialAsset;
        //         }
        //     }
        // }
        return nullptr;
    }

    MeshAsset* DCCAsset::FindMeshByName(const StringView& meshName)
    {
        // for (Asset* asset : GetChildren())
        // {
        //     if (MeshAsset* meshAsset = dynamic_cast<MeshAsset*>(asset))
        //     {
        //         if (meshAsset->GetName() == meshName)
        //         {
        //             return meshAsset;
        //         }
        //     }
        // }
        return nullptr;
    }

    TextureAsset* DCCAsset::FindTextureByName(const StringView& textureName)
    {
        // for (Asset* asset : GetChildren())
        // {
        //     if (TextureAsset* textureAsset = dynamic_cast<TextureAsset*>(asset))
        //     {
        //         if (textureAsset->GetName() == textureName)
        //         {
        //             return textureAsset;
        //         }
        //     }
        // }
        return nullptr;
    }

    SceneObjectAsset* DCCAsset::GetSceneObjectAsset()
    {
        // for (Asset* asset : GetChildren())
        // {
        //     if (SceneObjectAsset* sceneObjectAsset = dynamic_cast<SceneObjectAsset*>(asset))
        //     {
        //         return sceneObjectAsset;
        //     }
        // }
        return nullptr;
    }

    void DCCAssetImportSettings::RegisterType(NativeTypeHandler<DCCAssetImportSettings>& type)
    {
        type.Field<&DCCAssetImportSettings::scaleFactor>("scaleFactor");
        type.Field<&DCCAssetImportSettings::mergeMaterials>("mergeMaterials");
        type.Field<&DCCAssetImportSettings::mergeMeshes>("mergeMeshes");
    }

    void DCCAsset::RegisterType(NativeTypeHandler<DCCAsset>& type)
    {
    }
}
