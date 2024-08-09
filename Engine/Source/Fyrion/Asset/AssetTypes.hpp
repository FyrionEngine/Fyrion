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

        virtual Asset* CreateAsset()
        {
            return nullptr;
        }

        virtual void ImportAsset(StringView path, Asset* asset) {}

        virtual     ~AssetIO() = default;
        static void RegisterType(NativeTypeHandler<AssetIO>& type);
    };


    class FY_API AssetDirectory final : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        void BuildPath() override;

        void SetExtension(StringView p_extension) override {}

        StringView   GetDisplayName() const override;
        bool         IsModified() const override;
        void         OnCreated() override;

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
        Asset*           CreateAsset() override;
        void             ImportAsset(StringView path, Asset* asset) override;
    };
}
