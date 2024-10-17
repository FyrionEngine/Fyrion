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

    void Scene::RegisterType(NativeTypeHandler<Scene>& type)
    {

    }

    GameObject& Scene::GetRootObject()
    {
        return root;
    }
}
