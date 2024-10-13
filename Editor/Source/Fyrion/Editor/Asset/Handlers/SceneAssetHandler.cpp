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
    };

    void RegisterSceneAssetHandler()
    {
        Registry::Type<SceneAssetHandler>();
    }
}
