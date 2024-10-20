#include "Scene.hpp"

#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
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

    GameObject& Scene::GetRootObject()
    {
        return root;
    }

    ArchiveValue Scene::Serialize(ArchiveWriter& writer) const
    {
        ArchiveValue sceneValue = writer.CreateObject();
        writer.AddToObject(sceneValue, "root", root.Serialize(writer));
        return sceneValue;
    }

    void Scene::Deserialize(ArchiveReader& reader, ArchiveValue value)
    {
        root.Deserialize(reader, reader.GetObjectValue(value, "root"));
    }

    void Scene::RegisterType(NativeTypeHandler<Scene>& type)
    {

    }
}
