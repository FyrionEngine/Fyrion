#pragma once
#include "Fyrion/Core/Array.hpp"

namespace Fyrion
{
    struct AssetFile;

    struct FY_API AssetImporter
    {
        virtual ~AssetImporter() = default;

        virtual Array<String> ImportExtensions() = 0;
        virtual void          ImportAsset(StringView path) = 0;
    };

    struct FY_API AssetHandler
    {
        virtual ~AssetHandler() = default;
        virtual StringView Extension() = 0;
        virtual void Save(AssetFile * assetFile) = 0;
    };
}
