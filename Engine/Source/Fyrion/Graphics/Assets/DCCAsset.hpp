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
        void        AddMesh(MeshAsset* mesh);

    private:
        Vec3                            scaleFactor{1.0, 1.0, 1.0};
        bool                            mergeMaterials = true;
        bool                            mergeMeshes = false;
        Array<Subobject<TextureAsset>>  textures{};
        Array<Subobject<MeshAsset>>     meshes{};
        Array<Subobject<MaterialAsset>> materials{};
        Subobject<SceneObjectAsset>     sceneObject{};
    };
}
