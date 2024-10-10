#pragma once
#include "Fyrion/Core/Array.hpp"


namespace Fyrion
{
    class AssetEditor;

    struct FY_API AssetImporter
    {
        virtual ~AssetImporter() = default;

        virtual Array<String> ImportExtensions() = 0;
        virtual TypeID        GetImportSettings() = 0;
        virtual bool          ImportAsset(AssetEditor& assetEditor, StringView path, ConstPtr importSettings) = 0;
    };
}
