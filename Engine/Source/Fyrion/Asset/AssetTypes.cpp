#include "AssetTypes.hpp"

#include "AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    void AssetIO::RegisterType(NativeTypeHandler<AssetIO>& type)
    {

    }

    void AssetDirectory::RegisterType(NativeTypeHandler<AssetDirectory>& type)
    {
    }

    void AssetDirectory::BuildPath()
    {
        Asset::BuildPath();
        for (Asset* child : GetChildren())
        {
            child->BuildPath();
        }
    }

    // void AssetDirectory::OnActiveChanged()
    // {
    //     for (Asset* child : children)
    //     {
    //         child->SetActive(IsActive());
    //     }
    // }

    StringView AssetDirectory::GetDisplayName() const
    {
        return "Folder";
    }

    bool AssetDirectory::IsModified() const
    {
        return false;
    }

    void AssetDirectory::OnCreated()
    {
        FileSystem::CreateDirectory(GetAbsolutePath());
    }

    String AssetDirectory::GetCacheDirectory() const
    {
        //directories don't have cache directory.
        return {};
    }

    Array<u8> UIFontAsset::GetFont() const
    {
        if (usize size = GetCacheSize(fontBytes))
        {
            Array<u8> ret(size);
            LoadCache(fontBytes, ret.Data(), ret.Size());
            return ret;
        }
        return {};
    }

    void UIFontAsset::RegisterType(NativeTypeHandler<UIFontAsset>& type)
    {
        type.Field<&UIFontAsset::fontBytes>("fontBytes");
    }

    UIFontAssetIO::UIFontAssetIO()
    {
        getImportExtensions = GetImportExtensions;
        getAssetTypeId = GetAssetTypeID;
        importAsset = ImportAsset;
    }

    Span<StringView> UIFontAssetIO::GetImportExtensions()
    {
        return {extensions, 2};
    }

    TypeID UIFontAssetIO::GetAssetTypeID(StringView path)
    {
        return GetTypeID<UIFontAsset>();
    }

    void UIFontAssetIO::ImportAsset(StringView path, Asset* asset)
    {
        UIFontAsset* fontAsset = asset->Cast<UIFontAsset>();

        Array<u8> bytes = FileSystem::ReadFileAsByteArray(path);
        fontAsset->SaveCache(fontAsset->fontBytes, bytes.begin(), bytes.Size());
    }

    void RegisterAssetTypes()
    {
        Registry::Type<AssetIO>();
        Registry::Type<Asset>();
        Registry::Type<AssetDirectory>();
        Registry::Type<UIFontAsset>();
        Registry::Type<UIFontAssetIO>();
        Registry::Type<ImportSettings>();
    }
}
