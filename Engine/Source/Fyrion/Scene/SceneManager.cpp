#include "SceneManager.hpp"

#include "Component.hpp"
#include "SceneAssets.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceObject.hpp"

namespace Fyrion
{

    SceneGlobals::SceneGlobals(StringView name, RID asset) : m_rootObject(name, asset, this)
    {
    }

    void SceneGlobals::SceneObjectAdded(SceneObject* sceneObject)
    {
        if (sceneObject->m_asset)
        {
            m_objectsByRID.Insert(sceneObject->m_asset, sceneObject);
        }
    }

    void SceneGlobals::SceneObjectRemoved(SceneObject* sceneObject)
    {
        if (sceneObject->m_asset)
        {
            m_objectsByRID.Erase(sceneObject->m_asset);
        }
    }

    SceneObject* SceneGlobals::GetRootObject()
    {
        return &m_rootObject;
    }

    void SceneGlobals::EnqueueDestroy(SceneObject* sceneObject)
    {
        m_objectsToDestroy.emplace(sceneObject);
    }

    void SceneGlobals::EnqueueStart(Component* component)
    {
        m_componentsToStart.emplace(component);
    }

    void SceneGlobals::AddUpdatableComponent(Component* component)
    {
        component->m_updateIndex = m_updatables.Size();
        m_updatables.EmplaceBack(component);
    }

    void SceneGlobals::EnqueueDestroy(Component* component, TypeHandler* typeHandler)
    {
        m_enqueedToDestroy.emplace(component, typeHandler);
    }

    void SceneGlobals::RemoveUpdatableComponent(Component* component)
    {
        Component* lastComponent = m_updatables.Back();
        lastComponent->m_updateIndex = component->m_updateIndex;
        m_updatables[lastComponent->m_updateIndex] = lastComponent;
        component->m_updateIndex = U64_MAX;
        m_updatables.PopBack();
    }

    void SceneGlobals::DoUpdate(f64 deltaTime)
    {
        while (!m_componentsToStart.empty())
        {
            Component* component = m_componentsToStart.front();
            component->OnStart();
            component->m_started = true;
            m_componentsToStart.pop();
        }

        for (Component* component : m_updatables)
        {
            component->OnUpdate(deltaTime);
        }

        while (!m_enqueedToDestroy.empty())
        {
            Pair<Component*, TypeHandler*> component = m_enqueedToDestroy.front();
            component.second->Destroy(component.first);
            m_enqueedToDestroy.pop();
        }

        while (!m_objectsToDestroy.empty())
        {
            m_objectsToDestroy.front()->DestroyImmediate();
            m_objectsToDestroy.pop();
        }
    }

    namespace
    {
        SceneObject*                             currentScene{};
        HashMap<String, UniquePtr<SceneGlobals>> scenesByName{};
        HashMap<RID, UniquePtr<SceneGlobals>>    scenesByAsset{};
    }

    SceneObject* SceneManager::LoadScene(RID rid)
    {
        auto it = scenesByAsset.Find(rid);
        if (it == scenesByAsset.end())
        {
            ResourceObject asset = Repository::Read(rid);
            it = scenesByAsset.Emplace(rid, MakeUnique<SceneGlobals>(asset[Asset::Name].As<String>(), asset[Asset::Object].As<RID>())).first;
        }
        return it->second->GetRootObject();
    }

    SceneObject* SceneManager::CreateScene(const StringView& sceneName)
    {
        auto it = scenesByName.Find(sceneName);
        if (it == scenesByName.end())
        {
            it = scenesByName.Emplace(sceneName, MakeUnique<SceneGlobals>(sceneName, RID{})).first;
        }
        return it->second->GetRootObject();
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
            currentScene->GetSceneGlobals()->DoUpdate(deltaTime);
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
        scenesByName.Clear();
        scenesByAsset.Clear();
        currentScene = nullptr;
    }
}
