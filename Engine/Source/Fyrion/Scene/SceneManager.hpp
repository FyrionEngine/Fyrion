#pragma once

#include "SceneObject.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion::SceneManager
{
    FY_API void         Destroy(SceneObject* sceneObject);
    FY_API void         SetActiveObject(SceneObject* sceneObject);
    FY_API SceneObject* CreateObject();
    FY_API SceneObject* CreateObject(SceneObjectAsset* asset);
}
