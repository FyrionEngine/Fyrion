#include "JsonAssetHandler.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Graphics/Assets/MaterialAsset.hpp"

namespace Fyrion
{
    struct MaterialAssetHandler : JsonAssetHandler
    {
        FY_BASE_TYPES(JsonAssetHandler);

        StringView Extension() override
        {
            return ".material";
        }

        TypeID GetAssetTypeID() override
        {
            return GetTypeID<MaterialAsset>();
        }

        void OpenAsset(AssetFile* assetFile) override
        {
            MaterialAsset* materialAsset = Assets::Load<MaterialAsset>(assetFile->uuid);
        }

        Image GenerateThumbnail(AssetFile* assetFile) override
        {
            return {};
        }
    };

    void RegisterMaterialAssetHandler()
    {
        Registry::Type<MaterialAssetHandler>();
    }
}
