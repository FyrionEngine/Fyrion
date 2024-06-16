#include "Component.hpp"
#include "SceneObject.hpp"
#include "Components/RenderComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<Component>();
        Registry::Type<SceneObject>();
        Registry::Type<TransformComponent, Component>();
        Registry::Type<RenderComponent, Component>();
    }
}
