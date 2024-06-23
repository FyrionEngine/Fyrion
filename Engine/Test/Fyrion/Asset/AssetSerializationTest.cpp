#include "doctest.h"
#include "Fyrion/Asset/AssetSerialization.hpp"

using namespace Fyrion;

namespace
{

    TEST_CASE("AssetSerialization::Basics")
    {
        JsonAssetWriter writer{};
        writer.WriteInt("testInt", 10);
        writer.WriteString("testStr", "strstr");

        String str = writer.GetString();
        CHECK(!str.Empty());
    }

}