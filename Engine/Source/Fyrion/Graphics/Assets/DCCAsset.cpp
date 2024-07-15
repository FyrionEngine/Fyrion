#include "DCCAsset.hpp"

namespace Fyrion
{

    void DCCAsset::AddMesh(MeshAsset* mesh)
    {
        meshes.EmplaceBack(mesh);
    }

    void DCCAsset::RegisterType(NativeTypeHandler<DCCAsset>& type)
    {
        type.Field<&DCCAsset::scaleFactor>("scaleFactor");
        type.Field<&DCCAsset::mergeMaterials>("mergeMaterials");
        type.Field<&DCCAsset::mergeMeshes>("mergeMeshes");
        type.Field<&DCCAsset::textures>("textures");
        type.Field<&DCCAsset::meshes>("meshes");
        type.Field<&DCCAsset::materials>("materials");
        type.Field<&DCCAsset::sceneObject>("sceneObject");
    }
}
