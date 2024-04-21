#include "SceneManager.hpp"

#include <queue>

#include "SceneTypes.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceObject.hpp"

namespace Fyrion
{
    namespace
    {
        SceneObject*                            currentScene{};
        HashMap<String, UniquePtr<SceneObject>> scenesByName{};
        HashMap<RID, UniquePtr<SceneObject>>    scenesByAsset{};
        std::queue<SceneObject*>                objectsToDestroy{};
    }

    SceneObject* SceneManager::LoadScene(RID rid)
    {
        auto it = scenesByAsset.Find(rid);
        if (it == scenesByAsset.end())
        {
            ResourceObject asset = Repository::Read(rid);
            it = scenesByAsset.Emplace(rid, MakeUnique<SceneObject>(asset[Asset::Name].As<String>(), asset[Asset::Object].As<RID>(), nullptr)).first;
        }
        return it->second.Get();
    }

    SceneObject* SceneManager::CreateScene(const StringView& sceneName)
    {
        auto it = scenesByName.Find(sceneName);
        if (it == scenesByName.end())
        {
            it = scenesByName.Emplace(sceneName, MakeUnique<SceneObject>(sceneName, nullptr)).first;
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
            currentScene->DoStart();
            currentScene->DoUpdate(deltaTime);
        }

        while (!objectsToDestroy.empty())
        {
            objectsToDestroy.front()->DestroyImmediate();
            objectsToDestroy.pop();
        }
    }

    void SceneManager::EnqueueDestroy(SceneObject* sceneObject)
    {
        objectsToDestroy.emplace(sceneObject);
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
        scenesByName.Clear();
        scenesByAsset.Clear();
        currentScene = nullptr;
    }
}
