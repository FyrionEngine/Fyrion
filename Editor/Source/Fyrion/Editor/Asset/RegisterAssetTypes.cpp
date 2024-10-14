#include "AssetEditor.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Handlers/JsonAssetHandler.hpp"

namespace Fyrion
{
    void RegisterSceneAssetHandler();
    void RegisterTextureAssetHandler();

    void RegisterAssetTypes()
    {
        Registry::Type<AssetImporter>();
        Registry::Type<AssetHandler>();
        Registry::Type<JsonAssetHandler>();

        RegisterSceneAssetHandler();
        RegisterTextureAssetHandler();
    }
}
