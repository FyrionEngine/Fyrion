#include "Component.hpp"
#include "SceneObject.hpp"
#include "SceneAssets.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<Component>();
        Registry::Type<SceneObject>();

        ResourceTypeBuilder<SceneObjectAsset>::Builder("Fyrion::Scene")
            .Value<SceneObjectAsset::Name, String>("Name")
            .SubObjectSet<SceneObjectAsset::Components>("Components")
            .Value<SceneObjectAsset::Order, u64>("Order")
            .SubObjectSet<SceneObjectAsset::Children>("Children")
            .Build();
    }
}
