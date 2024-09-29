#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetManager.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/IO/Path.hpp"

using namespace Fyrion;

namespace
{
    TEST_CASE("Asset::LoadPackageDirectory")
    {
        Engine::Init();
        {
            String package = Path::Join(FY_TEST_FILES, "Assets", "PackageFolder");
            AssetManager::LoadPackage("PackageFolder", package);




        }
        Engine::Destroy();
    }
}
