#pragma once

#include "Fyrion/Common.hpp"
#include "ResourceTypes.hpp"
#include "Fyrion/Core/String.hpp"

namespace Fyrion::ResourceAssets
{
	FY_API void         AddAssetImporter(StringView extensions, FnImportAsset fnImportAsset);
	FY_API RID          LoadAssetsFromDirectory(const StringView& name, const StringView& directory);
	FY_API void         SaveAssetsToDirectory(RID rid, const StringView& directory);
	FY_API RID          GetAssetRootByName(const StringView& name);
	FY_API String       GetName(RID asset);
	FY_API RID          GetParent(RID asset);
    FY_API String       GetPath(RID rid);
	FY_API u32          GetLoadedVersion(RID rid);
	FY_API StringView   GetAbsolutePath(RID rid);
    FY_API void         ImportAsset(RID root, RID directory, const StringView& path);
}
