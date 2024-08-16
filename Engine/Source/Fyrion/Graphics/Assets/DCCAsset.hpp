#pragma once
#include "MaterialAsset.hpp"
#include "MeshAsset.hpp"
#include "TextureAsset.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Scene/SceneTypes.hpp"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"

namespace Fyrion
{

    struct DCCAssetImportSettings : ImportSettings
    {
        FY_BASE_TYPES(ImportSettings);

        TypeHandler* GetTypeHandler() override
        {
            return Registry::FindType<DCCAssetImportSettings>();
        }

        Vec3 scaleFactor{1.0, 1.0, 1.0};
        bool mergeMaterials = false;
        bool mergeMeshes = false;

        static void RegisterType(NativeTypeHandler<DCCAssetImportSettings>& type);
    };


    class FY_API DCCAsset : public Asset, public SceneObjectAssetProvider
    {
    public:
        FY_BASE_TYPES(Asset, SceneObjectAssetProvider);

        static void RegisterType(NativeTypeHandler<DCCAsset>& type);

        MaterialAsset*    FindMaterialByName(const StringView& materialName);
        MeshAsset*        FindMeshByName(const StringView& meshName);
        TextureAsset*     FindTextureByName(const StringView& textureName) const;
        SceneObjectAsset* GetSceneObjectAsset() override;

    private:
        DCCAssetImportSettings importSettings;
    };
}
