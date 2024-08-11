#pragma once
#include "Asset.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    typedef Span<StringView>(*FnGetImportExtensions)();
    typedef TypeID (*FnGetAssetTypeID)(StringView path);
    typedef void(*FnImportAsset)(StringView path, Asset* asset);
    typedef void(*FnRenameAsset)(Asset* asset, StringView newName);

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


    class FY_API AssetDirectory final : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        void        BuildPath() override;
        StringView  GetDisplayName() const override;
        bool        IsModified() const override;
        void        OnCreated() override;
        String      GetCacheDirectory() const override;
        static void RegisterType(NativeTypeHandler<AssetDirectory>& type);
    };

    struct UIFontAsset final : Asset
    {
        FY_BASE_TYPES(Asset);

        CacheRef fontBytes;

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
