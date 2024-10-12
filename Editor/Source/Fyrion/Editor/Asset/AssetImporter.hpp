#pragma once
#include "Fyrion/Core/Array.hpp"


namespace Fyrion
{
    struct FY_API AssetImporter
    {
        virtual ~AssetImporter() = default;

        virtual Array<String> ImportExtensions() = 0;
        virtual TypeID        GetImportSettings() = 0;
        virtual bool          ImportAsset(StringView path, ConstPtr importSettings) = 0;
    };
}
