#include "WordAsset.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{

    void WorldTypeRegister()
    {
        ResourceTypeBuilder<WorldAsset>::Builder("Fyrion::World")
            .SubObjectSet<WorldAsset::Entities>("Entities")
            .Build();

        ResourceTypeBuilder<EntityAsset>::Builder()
            .Value<EntityAsset::Name, String>("Name")
            .SubObjectSet<EntityAsset::Components>("Components")
            .Value<EntityAsset::Parent, String>("Parent")
            .SubObjectSet<EntityAsset::Children>("Entities")
            .Build();
    }
}
