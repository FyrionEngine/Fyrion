#pragma once
#include "Asset.hpp"

namespace Fyrion
{
    struct AssetIO
    {
        virtual Span<StringView> GetImportExtensions()
        {
            return {};
        }

        virtual Asset* ImportAsset(StringView path, Asset* reimportAsset)
        {
            return nullptr;
        }

        virtual     ~AssetIO() = default;
        static void RegisterType(NativeTypeHandler<AssetIO>& type);
    };


    struct AssetDirectory final : Asset
    {
        FY_BASE_TYPES(Asset);

        Subobject<Asset> children;

        static void RegisterType(NativeTypeHandler<AssetDirectory>& type);

        void BuildPath() override;
        void OnActiveChanged() override;
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
        Asset*           ImportAsset(StringView path, Asset* reimportAsset) override;
    };
}
