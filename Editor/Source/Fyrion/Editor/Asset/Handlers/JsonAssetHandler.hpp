#pragma once

#include "Fyrion/Core/Repository.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"

namespace Fyrion
{
    struct JsonAssetHandler : AssetHandler
    {
        FY_BASE_TYPES(AssetHandler);

        void Save(StringView newPath, AssetFile* assetFile) override;
        void Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance) override;
    };
}
