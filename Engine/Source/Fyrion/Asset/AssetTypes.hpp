#pragma once
#include "Asset.hpp"

namespace Fyrion
{
    struct AssetIO
    {
        virtual Array<String> GetImportExtensions()
        {
            return {};
        }

        virtual Asset* ImportAsset(StringView path, Asset* reimportAsset)
        {
            return nullptr;
        }

        virtual     ~AssetIO() = default;
        static void RegisterType(NativeTypeHandler<AssetIO>& type);
    };


    struct AssetDirectory : Asset
    {
        FY_BASE_TYPES(Asset);

        Subobject<Asset> children;

        static void RegisterType(NativeTypeHandler<AssetDirectory>& type);
    };
}
