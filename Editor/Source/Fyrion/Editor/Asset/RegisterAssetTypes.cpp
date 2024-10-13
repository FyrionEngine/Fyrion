#include "AssetEditor.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Handlers/JsonAssetHandler.hpp"

namespace Fyrion
{
    void RegisterTextureIO();
    void RegisterSceneAssetHandler();

    void RegisterAssetTypes()
    {
        Registry::Type<AssetImporter>();
        Registry::Type<AssetHandler>();
        Registry::Type<JsonAssetHandler>();

        RegisterTextureIO();
        RegisterSceneAssetHandler();
    }
}
