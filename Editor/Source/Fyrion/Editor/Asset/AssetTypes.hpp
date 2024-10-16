#pragma once
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    struct AssetFile;

    struct FY_API AssetImporter
    {
        virtual ~AssetImporter() = default;

        virtual Array<String> ImportExtensions() = 0;
        virtual void          ImportAsset(AssetFile* parent, StringView path) = 0;
    };

    struct FY_API AssetHandler
    {
        virtual            ~AssetHandler() = default;
        virtual StringView Extension() = 0;
        virtual TypeID     GetAssetTypeID() = 0;
        virtual void       Save(StringView newPath, AssetFile* assetFile) = 0;
        virtual void       Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance) = 0;
        virtual void       OpenAsset(AssetFile* assetFile) = 0;
        virtual Image      GenerateThumbnail(AssetFile* assetFile) = 0;
    };
}
