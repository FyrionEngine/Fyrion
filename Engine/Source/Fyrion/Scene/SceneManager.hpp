#pragma once

#include "SceneObject.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    class FY_API SceneManager
    {
    public:
        static SceneObject* LoadScene(RID rid);
        static SceneObject* CreateScene(const StringView& sceneName);
        static SceneObject* GetCurrentScene();
        static void         SetCurrenScene(SceneObject* newCurrentScene);

        static void RegisterType(NativeTypeHandler<SceneManager>& type);

        friend class SceneObject;
    private:
        static void ExecuteUpdate(f64 deltaTime);
        static void EnqueueDestroy(SceneObject* sceneObject);
    };
}
