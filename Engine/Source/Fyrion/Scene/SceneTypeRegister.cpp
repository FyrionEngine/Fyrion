#include "Component.hpp"
#include "SceneObject.hpp"
#include "SceneAssets.hpp"
#include "Components/RenderComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<Component>();
        Registry::Type<SceneObject>();
        Registry::Type<TransformComponent, Component>();
        Registry::Type<RenderComponent, Component>();

        ResourceTypeBuilder<SceneObjectAsset>::Builder("Fyrion::Scene")
            .Value<SceneObjectAsset::name, String>("name")
            .SubObjectSet<SceneObjectAsset::components>("components")
            .SubObjectSet<SceneObjectAsset::children>("children")
            .Value<SceneObjectAsset::childrenSort, Array<RID>>("childrenSort")
            .Build();
    }
}
