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
            .Value<SceneObjectAsset::Name, String>("Name")
            .SubObjectSet<SceneObjectAsset::Components>("Components")
            .SubObjectSet<SceneObjectAsset::Children>("Children")
            .Value<SceneObjectAsset::ChildrenSort, Array<RID>>("ChildrenSort")
            .SubObjectSet<SceneObjectAsset::SubObjects>("SubObjects")
            .Build();
    }
}
