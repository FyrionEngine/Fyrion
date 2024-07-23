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
        void        SetImage(const Image& image);
        Texture     GetTexture();
        static void RegisterType(NativeTypeHandler<TextureAsset>& type);

    private:
        u32  width = 0;
        u32  height = 0;
        u32  channels = 0;
        Blob data{};

        Texture texture{};
    };
}
