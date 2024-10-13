#include "Scene.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterSceneTypes()
    {
        Registry::Type<Scene>();
    }
}
