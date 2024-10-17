#include "Scene.hpp"

#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    ArchiveValue Scene::Serialize(ArchiveWriter& writer) const
    {
        return {};
    }

    void Scene::Deserialize(ArchiveReader& reader, ArchiveValue value)
    {

    }

    void Scene::DestroyGameObject(GameObject* gameObject)
    {
        queueToDestroy.EmplaceBack(gameObject);
    }

    void Scene::FlushQueues()
    {
        for (GameObject* gameObject : queueToDestroy)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(gameObject);
        }
        queueToDestroy.Clear();
    }

    void Scene::DoUpdate()
    {


        FlushQueues();
    }

    void Scene::RegisterType(NativeTypeHandler<Scene>& type)
    {

    }

    GameObject& Scene::GetRootObject()
    {
        return root;
    }
}
