#pragma once

#include "StringView.hpp"
#include "String.hpp"
#include "Image.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion::StaticContent
{
    FY_API Array<u8> GetBinaryFile(StringView path);
    FY_API String    GetTextFile(StringView path);
    FY_API Image     GetImageFile(StringView path);
    FY_API Texture   GetTextureFile(StringView path);
}
