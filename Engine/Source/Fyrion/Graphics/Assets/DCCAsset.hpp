#pragma once
#include "MaterialAsset.hpp"
#include "MeshAsset.hpp"
#include "TextureAsset.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"

namespace Fyrion
{
    class FY_API DCCAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<DCCAsset>& type);
        void        AddMesh(MeshAsset* meshAsset);
        void        AddTexture(TextureAsset* textureAsset);
        void        AddMaterial(MaterialAsset* materialAsset);
        void        AddSceneObject(SceneObjectAsset* sceneObjectAsset);

    private:
        Vec3                         scaleFactor{1.0, 1.0, 1.0};
        bool                         mergeMaterials = true;
        bool                         mergeMeshes = false;
        Subobjects<TextureAsset>     textures{this};
        Subobjects<MeshAsset>        meshes{this};
        Subobjects<MaterialAsset>    materials{this};
        Subobjects<SceneObjectAsset> sceneObjects{this};
    };
}
