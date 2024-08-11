#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    enum class TextureType
    {
        Texture2D,
        Texture3D,
        Cubemap
    };

    struct FY_API TextureImportSettings : ImportSettings
    {
        TypeHandler* GetTypeHandler() override;

        bool        generateMipmaps = true;
        TextureType textureType = TextureType::Texture2D;

        static void RegisterType(NativeTypeHandler<TextureImportSettings>& type);
    };

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

        void       SetImagePath(StringView path);
        void       SetHDRImagePath(StringView path);
        void       SetImage(const Image& image);
        void       SetHDRImage(const HDRImage& image);
        Texture    CreateTexture() const;
        Texture    GetTexture();
        Sampler    GetSampler();
        Image      GetImage() const;
        Format     GetFormat() const;

        void SetTextureType(TextureType textureType);

        static void RegisterType(NativeTypeHandler<TextureAsset>& type);

    private:
        TextureImportSettings textureImportSettings{};
        Format format{Format::RGBA};

        u32                      mipLevels{};
        u32                      arrayLayers{};
        String                   imageBuffer{};
        Array<TextureAssetImage> images{};

        Texture texture{};
        Sampler sampler{};

        CacheRef textureData{};
    };
}
