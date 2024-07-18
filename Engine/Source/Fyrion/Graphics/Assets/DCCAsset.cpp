#include "DCCAsset.hpp"

namespace Fyrion
{
    void DCCAsset::AddMesh(MeshAsset* meshAsset)
    {
        meshes.Add(meshAsset);
    }

    void DCCAsset::AddTexture(TextureAsset* textureAsset)
    {
        textures.Add(textureAsset);
    }

    void DCCAsset::AddMaterial(MaterialAsset* materialAsset)
    {
        materials.Add(materialAsset);
    }
    void DCCAsset::AddSceneObject(SceneObjectAsset* sceneObjectAsset)
    {
        sceneObjects.Add(sceneObjectAsset);
    }

    void DCCAsset::RegisterType(NativeTypeHandler<DCCAsset>& type)
    {
        type.Field<&DCCAsset::scaleFactor>("scaleFactor");
        type.Field<&DCCAsset::mergeMaterials>("mergeMaterials");
        type.Field<&DCCAsset::mergeMeshes>("mergeMeshes");
        type.Field<&DCCAsset::textures>("textures");
        type.Field<&DCCAsset::meshes>("meshes");
        type.Field<&DCCAsset::materials>("materials");
        type.Field<&DCCAsset::sceneObjects>("sceneObjects");
    }
}
