#include "SceneManager.hpp"

#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/UniquePtr.hpp"

namespace Fyrion
{
    namespace
    {
        SceneObject*                            currentScene{};
        HashMap<String, UniquePtr<SceneObject>> scenes{};
    }

    SceneObject* SceneManager::FindOrCreateScene(const StringView& sceneName)
    {
        auto it = scenes.Find(sceneName);
        if (it == scenes.end())
        {
            it = scenes.Emplace(sceneName, MakeUnique<SceneObject>(nullptr, sceneName)).first;
        }
        return it->second.Get();
    }

    SceneObject* SceneManager::GetCurrentScene()
    {
        return currentScene;
    }

    void SceneManager::SetCurrenScene(SceneObject* newCurrentScene)
    {
        currentScene = newCurrentScene;
    }

    void SceneManagerInit()
    {

    }

    void SceneManagerShutdown()
    {
        scenes.Clear();
        currentScene = nullptr;
    }
}
