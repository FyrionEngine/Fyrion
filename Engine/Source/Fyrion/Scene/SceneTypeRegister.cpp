#include "SceneTypes.hpp"
#include "Assets/SceneObjectAsset.hpp"
#include "Components/Light.hpp"
#include "Components/MeshRender.hpp"
#include "Components/SceneEnvironment.hpp"
#include "Components/TransformComponent.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<SceneObjectAssetProvider>();
        Registry::Type<SceneObjectAsset>();
        Registry::Type<Component>();
        Registry::Type<TransformComponent>();
        Registry::Type<SceneObject>();
        Registry::Type<MeshRender>();
        Registry::Type<SceneEnvironment>();
        Registry::Type<Light>();
    }
}
