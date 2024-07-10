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

        virtual Asset* ImportAsset(StringView path, Asset* reimportAsset)
        {
            return nullptr;
        }

        virtual     ~AssetIO() = default;
        static void RegisterType(NativeTypeHandler<AssetIO>& type);
    };


    class FY_API AssetDirectory final : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<AssetDirectory>& type);

        void BuildPath() override;
        void OnActiveChanged() override;

        void SetExtension(StringView p_extension) override {}

        StringView   GetDisplayName() const override;
        void         AddChild(Asset* child);
        void         RemoveChild(Asset* child);
        Span<Asset*> GetChildren();

    private:
        Array<Asset*> children;
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
