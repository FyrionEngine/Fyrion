#pragma once
#include "Fyrion/Core/Array.hpp"


namespace Fyrion
{
    struct FY_API AssetImporter
    {
        virtual ~AssetImporter() = default;

        virtual Array<String> ImportExtensions() = 0;
        virtual void          ImportAsset(StringView path) = 0;
    };

    struct FY_API AssetHandler
    {
        virtual ~AssetHandler() = default;

        virtual TypeID     GetAssetType() = 0;
        virtual StringView Extension() = 0;
        virtual void       SaveAsset(AssetFile* assetFile) = 0;
        virtual void       LoadAsset(AssetFile* assetFile) = 0;
    };
}
