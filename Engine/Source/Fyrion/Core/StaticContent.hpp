#pragma once

#include "StringView.hpp"
#include "String.hpp"
#include "Image.hpp"

namespace Fyrion::StaticContent
{
    Array<u8> GetBinaryFile(StringView path);
    String    GetTextFile(StringView path);
    Image     GetImageFile(StringView path);
}
