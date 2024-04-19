
#include "Component.hpp"
#include "SceneObject.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<Component>();
        Registry::Type<SceneObject>();
    }
}
