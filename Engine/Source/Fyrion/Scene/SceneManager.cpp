#include "SceneManager.hpp"

#include <queue>

#include "SceneObject.hpp"
#include "Assets/SceneObjectAsset.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Allocator.hpp"


namespace Fyrion
{
    class RenderGraphAsset;

    namespace
    {
        std::queue<SceneObject*> objectsToDestroy; //TODO(60457169) custom queue
        SceneObject*             activeSceneObject = nullptr;

        void DestroySceneObject(SceneObject* sceneObject)
        {
            if (sceneObject->GetParent() != nullptr)
            {
                sceneObject->GetParent()->RemoveChild(sceneObject);
            }
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(sceneObject);
        }

        void DoUpdate(f64 deltaTime)
        {
            while (!objectsToDestroy.empty())
            {
                DestroySceneObject(objectsToDestroy.front());
                objectsToDestroy.pop();
            }

            if (activeSceneObject)
            {
                //DO UPDATE.
            }
        }
    }

    void SceneManager::Destroy(SceneObject* sceneObject)
    {
        objectsToDestroy.emplace(sceneObject);
    }

    SceneObject* SceneManager::CreateObject()
    {
        return MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>();
    }

    SceneObject* SceneManager::CreateObjectFromAsset(SceneObjectAsset* asset)
    {
        SceneObject* object = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>();
        if (asset)
        {
            object->SetPrototype(asset->GetObject());
        }
        return object;
    }

    void SceneManager::SetActiveObject(SceneObject* sceneObject)
    {
        activeSceneObject = sceneObject;
    }

    void SceneManagerInit()
    {
        Event::Bind<OnUpdate, DoUpdate>();
    }

    void SceneManagerShutdown()
    {
        activeSceneObject = nullptr;
    }
}
