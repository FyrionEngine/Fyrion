#include "AssetTypes.hpp"

#include "AssetManager.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    void AssetIO::RegisterType(NativeTypeHandler<AssetIO>& type)
    {

    }

    Span<AssetInfo*> DirectoryInfo::GetChildren() const
    {
        return children;
    }

    void DirectoryInfo::AddChild(AssetInfo* metaInfo)
    {
        children.EmplaceBack(metaInfo);
        //TODO - sort
    }

    StringView DirectoryInfo::GetDisplayName() const
    {
        return "Folder";
    }

    void DirectoryInfo::UpdatePath()
    {
        AssetInfo::UpdatePath();
        for (AssetInfo* child : children)
        {
            child->UpdatePath();
        }
    }

    AssetInfo* DirectoryInfo::FindChildByAbsolutePath(StringView absolutePath) const
    {
        for (AssetInfo* child : children)
        {
            if (child->GetAbsolutePath() == absolutePath)
            {
                return child;
            }
        }
        return nullptr;
    }

    String DirectoryInfo::GetCacheDirectory() const
    {
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
        Registry::Type<DirectoryInfo>();
        Registry::Type<UIFontAsset>();
        Registry::Type<UIFontAssetIO>();
        Registry::Type<ImportSettings>();
    }
}
