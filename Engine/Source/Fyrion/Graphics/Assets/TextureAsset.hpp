#pragma once

#include "Fyrion/Common.hpp"

#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/IO/Asset.hpp"

namespace Fyrion
{
    class FY_API TextureAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        ~TextureAsset();
        void SetProperties(u32 width, u32 height, Format format);

        Texture GetTexture();
        Image GetImage() const;

        static void RegisterType(NativeTypeHandler<TextureAsset>& type);

    private:
        Extent3D     extent;
        Format       format = Format::Undefined;

        Texture texture = {};
    };
}
