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

        MaterialAsset*    FindMaterialByName(const StringView& materialName);
        MeshAsset*        FindMeshByName(const StringView& meshName);
        TextureAsset*     FindTextureByName(const StringView& textureName);
        SceneObjectAsset* GetSceneObjectAsset();

    private:
        Vec3 scaleFactor{1.0, 1.0, 1.0};
        bool mergeMaterials = false;
        bool mergeMeshes = false;
    };
}
