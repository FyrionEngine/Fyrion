#include "DCCAsset.hpp"

#include "Fyrion/Asset/AssetHandler.hpp"

namespace Fyrion
{
    MaterialAsset* DCCAsset::FindMaterialByName(const StringView& materialName)
    {
        for (AssetHandler* handler : GetHandler()->GetChildren())
        {
            if (handler->GetName() == materialName)
            {
                if (MaterialAsset* materialAsset = dynamic_cast<MaterialAsset*>(handler->LoadInstance()))
                {
                    return materialAsset;
                }
            }
        }
        return nullptr;
    }

    MeshAsset* DCCAsset::FindMeshByName(const StringView& meshName)
    {
        for (AssetHandler* handler : GetHandler()->GetChildren())
        {
            if (handler->GetName() == meshName)
            {
                if (MeshAsset* meshAsset = dynamic_cast<MeshAsset*>(handler->LoadInstance()))
                {
                    return meshAsset;
                }
            }
        }
        return nullptr;
    }

    TextureAsset* DCCAsset::FindTextureByName(const StringView& textureName) const
    {
        for (AssetHandler* handler : GetHandler()->GetChildren())
        {
            if (handler->GetName() == textureName)
            {
                if (TextureAsset* textureAsset = dynamic_cast<TextureAsset*>(handler->LoadInstance()))
                {
                    return textureAsset;
                }
            }
        }
        return nullptr;
    }

    SceneObjectAsset* DCCAsset::GetSceneObjectAsset()
    {
        for (AssetHandler* handler : GetHandler()->GetChildren())
        {
            if (SceneObjectAsset* sceneObjectAsset = dynamic_cast<SceneObjectAsset*>(handler->LoadInstance()))
            {
                return sceneObjectAsset;
            }
        }
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
