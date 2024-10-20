#include "JsonAssetHandler.hpp"
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
