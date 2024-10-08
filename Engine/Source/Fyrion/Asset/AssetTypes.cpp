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

    Array<u8> UIFontAsset::GetFont() const
    {
        return LoadBuffer(fontBytes);
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

    bool UIFontAssetIO::ImportAsset(StringView path, Asset* asset)
    {
        UIFontAsset* fontAsset = asset->Cast<UIFontAsset>();

        Array<u8> bytes = FileSystem::ReadFileAsByteArray(path);
        fontAsset->SaveBuffer(fontAsset->fontBytes, bytes.begin(), bytes.Size());

        return true;
    }

    void RegisterAssetTypes()
    {
        Registry::Type<AssetIO>();
        Registry::Type<Asset>();
        Registry::Type<UIFontAsset>();
        Registry::Type<UIFontAssetIO>();
        Registry::Type<ImportSettings>();
        Registry::Type<AssetMeta>();
    }
}
