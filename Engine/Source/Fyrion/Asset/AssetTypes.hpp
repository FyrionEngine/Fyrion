#pragma once
#include "Asset.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    struct AssetIO
    {
        virtual Span<StringView> GetImportExtensions()
        {
            return {};
        }

        virtual TypeID GetAssetTypeID(StringView path)
        {
            return 0;
        }

        virtual void ImportAsset(StringView path, Asset* asset) {}

        virtual     ~AssetIO() = default;
        static void RegisterType(NativeTypeHandler<AssetIO>& type);
    };

    struct ImportSettings
    {
        virtual TypeHandler* GetTypeHandler() = 0;
    };


    class FY_API AssetDirectory final : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        void       BuildPath() override;
        void       SetName(StringView name) override;
        StringView GetDisplayName() const override;
        bool       IsModified() const override;
        void       OnCreated() override;
        String     GetCacheDirectory() const override;

        static void RegisterType(NativeTypeHandler<AssetDirectory>& type);
    };

    struct UIFontAsset final : Asset
    {
        FY_BASE_TYPES(Asset);

        Array<u8> fontBytes;

        static void RegisterType(NativeTypeHandler<UIFontAsset>& type);
    };

    struct UIFontAssetIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        StringView extensions[2] = {".ttf", ".otf"};

        Span<StringView> GetImportExtensions() override;
        TypeID           GetAssetTypeID(StringView path) override;
        void             ImportAsset(StringView path, Asset* asset) override;
    };
}
