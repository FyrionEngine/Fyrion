#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class Component;
    class SceneObjectAsset;

    enum SceneNotifications_
    {
        SceneNotifications_None               = 0,
        SceneNotifications_OnActivated        = 1000,
        SceneNotifications_OnDeactivated      = 1010,
        SceneNotifications_OnComponentAdded   = 1020,
        SceneNotifications_OnComponentRemoved = 1030,

        SceneNotifications_TransformChanged       = 2000,
        SceneNotifications_ParentTransformChanged = 2010
    };

    struct SceneObjectAssetProvider
    {
        virtual SceneObjectAsset* GetSceneObjectAsset() = 0;
        virtual                   ~SceneObjectAssetProvider() = default;
    };

    struct NotificationEvent
    {
        i64    type;
        TypeID typeId;

        union
        {
            Component* component;
            VoidPtr    customData;
        };
    };
}
