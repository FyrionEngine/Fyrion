#include "AssetTypes.hpp"

#include "AssetDatabase.hpp"
#include "Fyrion/IO/FileSystem.hpp"


namespace Fyrion
{
    void AssetIO::RegisterType(NativeTypeHandler<AssetIO>& type)
    {
        type.Function<&AssetIO::GetImportExtensions>("GetImportExtensions");
        type.Function<&AssetIO::ImportAsset>("ImportAsset");
    }

    void AssetDirectory::RegisterType(NativeTypeHandler<AssetDirectory>& type)
    {
        type.Field<&AssetDirectory::children>("children");
    }

    void UIFontAsset::RegisterType(NativeTypeHandler<UIFontAsset>& type)
    {
        type.Field<&UIFontAsset::fontBytes>("fontBytes");
    }

    Span<StringView> UIFontAssetIO::GetImportExtensions()
    {
        return {extensions, 2};
    }

    Asset* UIFontAssetIO::ImportAsset(StringView path, Asset* reimportAsset)
    {
        UIFontAsset* fontAsset = AssetDatabase::Create<UIFontAsset>();
        fontAsset->fontBytes = FileSystem::ReadFileAsByteArray(path);
        return fontAsset;
    }

    void RegisterAssetTypes()
    {
        Registry::Type<AssetIO>();
        Registry::Type<Asset>();
        Registry::Type<AssetDirectory>();
        Registry::Type<UIFontAsset>();
        Registry::Type<UIFontAssetIO>();
    }
}
