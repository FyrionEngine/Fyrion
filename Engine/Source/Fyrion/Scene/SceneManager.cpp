#include "SceneManager.hpp"

#include <queue>

#include "SceneObject.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Allocator.hpp"


namespace Fyrion
{
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

    SceneObject* SceneManager::CreateObject(SceneGlobals* globals)
    {
        return MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>(globals);
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
