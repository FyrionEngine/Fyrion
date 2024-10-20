#include "AssetEditor.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Handlers/JsonAssetHandler.hpp"
#include "Handlers/MeshAssetHandler.hpp"

namespace Fyrion
{
    void RegisterSceneAssetHandler();
    void RegisterTextureAssetHandler();
    void RegisterShaderAssetHandlers();
    void RegisterGLTFImporter();
    void RegisterMaterialAssetHandler();

    void RegisterAssetTypes()
    {
        Registry::Type<AssetImporter>();
        Registry::Type<AssetHandler>();
        Registry::Type<JsonAssetHandler>();
        Registry::Type<MeshAssetHandler>();

        RegisterSceneAssetHandler();
        RegisterTextureAssetHandler();
        RegisterShaderAssetHandlers();
        RegisterGLTFImporter();
        RegisterMaterialAssetHandler();
    }
}
