#include "AssetEditor.hpp"
#include "AssetImporter.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "IOs/ImportSettings.hpp"

namespace Fyrion
{
    void RegisterTextureIO();

    void RegisterAssetTypes()
    {
        Registry::Type<AssetEditor>();
        Registry::Type<AssetImporter>();

        RegisterTextureIO();
    }
}
