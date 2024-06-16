#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"

using namespace Fyrion;

namespace
{
    TEST_CASE("Asset::Basic")
    {
        Engine::Init();
        {
            AssetDirectory* directory = AssetDatabase::Instantiate<AssetDirectory>();

            for (Asset* asset : directory->GetChildren())
            {
                asset->GetAssetType()
            }
        }
        Engine::Destroy();
    }
}
