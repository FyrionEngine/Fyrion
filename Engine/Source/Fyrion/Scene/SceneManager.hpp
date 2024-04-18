#pragma once

#include "SceneObject.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    class FY_API SceneManager
    {
    public:
        static SceneObject* FindOrCreateScene(const StringView& sceneName);
        static SceneObject* GetCurrentScene();
        static void         SetCurrenScene(SceneObject* newCurrentScene);
    };
}
