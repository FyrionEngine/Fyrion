#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/Scene.hpp"

namespace Fyrion
{
    struct SceneAssetHandler : JsonAssetHandler
    {
        FY_BASE_TYPES(JsonAssetHandler);

        StringView Extension() override
        {
            return ".scene";
        }

        TypeID GetAssetTypeID() override
        {
            return GetTypeID<Scene>();
        }

        void OpenAsset(AssetFile* assetFile) override
        {
            //Scene* scene = static_cast<Scene*>(Repository::Load(assetFile->uuid));
        }
    };

    void RegisterSceneAssetHandler()
    {
        Registry::Type<SceneAssetHandler>();
    }
}
