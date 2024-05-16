
#include "AssetTypes.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"
#include "Fyrion/IO/FileSystem.hpp"

namespace Fyrion
{
	RID ImportFontAsset(RID asset, const StringView& path)
	{
        Array<u8> bytes = FileSystem::ReadFileAsByteArray(path);
		RID rid = Repository::CreateResource<UIFont>();
		ResourceObject fontObject = Repository::Write(rid);
		fontObject[UIFont::FontBytes] =  bytes;
		fontObject.Commit();

		return rid;
	}

	void RegisterFontAsset()
	{
        ResourceTypeBuilder<UIFont>::Builder()
            .Value<UIFont::FontBytes, Array<u8>>("fontBytes")
            .Build();
        ResourceAssets::AddAssetImporter(".ttf,.otf", ImportFontAsset);
	}

}