#pragma once
#include "Asset.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    typedef Span<StringView>(*FnGetImportExtensions)();
    typedef TypeID (*FnGetAssetTypeID)(StringView path);
    typedef void(*FnImportAsset)(StringView path, Asset* asset);
    typedef void(*FnRenameAsset)(Asset* asset, StringView newName);

    struct AssetMeta
    {
        StringView displayName;
        TypeID     importSettings;
    };

    struct AssetIO
    {
        virtual ~AssetIO() = default;

        FnGetImportExtensions getImportExtensions = nullptr;
        FnGetAssetTypeID      getAssetTypeId = nullptr;
        FnImportAsset         importAsset = nullptr;
        FnRenameAsset         renameAsset = nullptr;

        static void RegisterType(NativeTypeHandler<AssetIO>& type);
    };

    struct ImportSettings
    {
        virtual              ~ImportSettings() = default;
        virtual TypeHandler* GetTypeHandler() = 0;
    };

    struct UIFontAsset final : Asset
    {
        FY_BASE_TYPES(Asset);

        AssetBuffer fontBytes;

        Array<u8> GetFont() const;

        static void RegisterType(NativeTypeHandler<UIFontAsset>& type);
    };

    struct UIFontAssetIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);
        UIFontAssetIO();

        static inline StringView extensions[2] = {".ttf", ".otf"};
        static Span<StringView> GetImportExtensions();
        static TypeID           GetAssetTypeID(StringView path);
        static void             ImportAsset(StringView path, Asset* asset);
    };
}
