#include "AssetEditor.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterTextureIO();

    void RegisterAssetTypes()
    {
        Registry::Type<AssetImporter>();
        Registry::Type<AssetHandler>();

        RegisterTextureIO();
    }
}
