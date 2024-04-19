#include "SceneManager.hpp"

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/UniquePtr.hpp"

namespace Fyrion
{
    namespace
    {
        SceneObject*                            currentScene{};
        HashMap<String, UniquePtr<SceneObject>> scenes{};
    }

    SceneObject* SceneManager::CreateScene(const StringView& sceneName)
    {
        auto it = scenes.Find(sceneName);
        if (it == scenes.end())
        {
            it = scenes.Emplace(sceneName, MakeUnique<SceneObject>(sceneName, nullptr, RID{})).first;
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

    void SceneManager::ExecuteUpdate(f64 deltaTime)
    {
        if (currentScene)
        {
            currentScene->DoUpdate(deltaTime);
        }
    }

    void SceneManager::RegisterType(NativeTypeHandler<SceneManager>& type)
    {
        Event::Bind<OnUpdate, ExecuteUpdate>();
    }

    void SceneManagerInit()
    {
        Registry::Type<SceneManager>();
    }

    void SceneManagerShutdown()
    {
        scenes.Clear();
        currentScene = nullptr;
    }
}
