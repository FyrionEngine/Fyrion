#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    struct TextureAssetImage
    {
        u32    byteOffset{};
        u32    mip{};
        u32    arrayLayer{};
        Extent extent{};
        usize  size{};

        static void RegisterType(NativeTypeHandler<TextureAssetImage>& type);
    };


    class FY_API TextureAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        ~TextureAsset() override;

        StringView GetDisplayName() const override;
        void       SetImagePath(StringView path);
        void       SetHDRImagePath(StringView path);
        void       SetImage(const Image& image);
        void       SetHDRImage(const HDRImage& image);
        Texture    CreateTexture() const;
        Texture    GetTexture();
        Sampler    GetSampler();
        Image      GetImage() const;
        Format     GetFormat() const;

        static void RegisterType(NativeTypeHandler<TextureAsset>& type);

    private:
        Format format{Format::RGBA};
        bool   generateMipmaps = true;
        bool   generateCubemap = false;

        u32                      mipLevels{};
        u32                      arrayLayers{};
        String                   imageBuffer{};
        Array<TextureAssetImage> images{};

        Texture                  texture{};
        Sampler                  sampler{};


        Blob textureData{};
        Blob cubeMapData{};

        void GenerateMipmaps();
    };
}
