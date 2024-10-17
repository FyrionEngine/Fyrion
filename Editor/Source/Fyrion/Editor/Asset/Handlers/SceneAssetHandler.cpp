#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Scene/SceneEditor.hpp"
#include "Fyrion/IO/Path.hpp"
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
            Editor::GetSceneEditor().SetScene(assetFile);
        }

        Image GenerateThumbnail(AssetFile* assetFile) override
        {
            return {};
        }
    };

    void RegisterSceneAssetHandler()
    {
        Registry::Type<SceneAssetHandler>();
    }
}
