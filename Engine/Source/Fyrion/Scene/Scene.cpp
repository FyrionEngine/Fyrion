#include "Scene.hpp"

#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void Scene::RegisterType(NativeTypeHandler<Scene>& type)
    {
        type.Field<&Scene::testStr>("testStr");
    }
}
