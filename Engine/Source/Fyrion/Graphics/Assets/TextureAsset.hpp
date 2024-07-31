#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    class FY_API TextureAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        ~TextureAsset() override;

        StringView  GetDisplayName() const override;
        void        SetImagePath(StringView path);
        void        SetHDRImagePath(StringView path);
        void        SetImage(const Image& image);
        void        SetHDRImage(const HDRImage& image);
        Texture     GetTexture();
        Image       GetImage() const;
        Format      GetFormat() const;
        static void RegisterType(NativeTypeHandler<TextureAsset>& type);

    private:
        u32    width = 0;
        u32    height = 0;
        u32    channels = 0;
        Format format = Format::RGBA;
        Blob   data{};
        Texture texture{};
    };
}
