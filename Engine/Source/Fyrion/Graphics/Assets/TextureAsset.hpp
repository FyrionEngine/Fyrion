#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StreamBuffer.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    class FY_API TextureAsset
    {
    public:
        ~TextureAsset();
        void SetData(u8* bytes, u32 width, u32 height, Format format);

        Texture GetTexture();

        static void RegisterType(NativeTypeHandler<TextureAsset>& type);


    private:
        Extent3D     extent;
        Format       format = Format::Undefined;
        StreamBuffer textureData;

        Texture texture = {};
    };
}
