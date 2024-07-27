#pragma once


namespace Fyrion
{
    class SceneObjectAsset;

    enum SceneNotifications_
    {
        SceneNotifications_OnActivated          = 1000,
        SceneNotifications_OnDeactivated        = 1010,
        SceneNotifications_OnComponentAdded     = 1020, //execute to all components in the scene object
        SceneNotifications_OnComponentRemoved   = 1030, //execute to all components in the scene object
        SceneNotifications_OnComponentCreated   = 1040, //execute only in the component created
        SceneNotifications_OnComponentDestroyed = 1050, //execute only in the component destroyed

        SceneNotifications_TransformChanged = 2000
    };

    struct SceneObjectAssetProvider
    {
        virtual SceneObjectAsset* GetSceneObjectAsset() = 0;
        virtual                   ~SceneObjectAssetProvider() = default;
    };
}
