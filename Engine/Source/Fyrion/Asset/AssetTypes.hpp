#pragma once
#include "Asset.hpp"

namespace Fyrion
{
    struct AssetDirectory : Asset
    {
        FY_BASE_TYPES(Asset);

        Subobject<Asset> children;

        static void RegisterType(NativeTypeHandler<AssetDirectory>& type);
    };
}
