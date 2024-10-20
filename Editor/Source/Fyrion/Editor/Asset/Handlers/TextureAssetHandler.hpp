#pragma once

namespace Fyrion
{
    class TextureAsset;
}


namespace Fyrion::TextureImporter
{
    void ImportTexture(AssetFile* assetFile, TextureAsset* textureAsset, Span<const u8> imageBuffer);
}