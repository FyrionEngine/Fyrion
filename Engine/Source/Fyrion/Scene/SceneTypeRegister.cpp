#include "Assets/SceneObjectAsset.hpp"
#include "Components/TransformComponent.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<SceneObjectAsset>();
        Registry::Type<Component>();
        Registry::Type<TransformComponent>();
        Registry::Type<SceneObject>();
    }
}
