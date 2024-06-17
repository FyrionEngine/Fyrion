#pragma once
#include "Asset.hpp"

namespace Fyrion
{
    struct AssetDirectory : Asset
    {
        Subobject children;
    };
}
