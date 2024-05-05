#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct UIFont
    {
        constexpr static u32 fontBytes = 0;
    };

    struct TextureAsset
    {
        constexpr static u32 extent = 0;
        constexpr static u32 channels = 1;
        constexpr static u32 data = 2;
    };

    struct ShaderAsset
    {
        constexpr static u32 bytes = 0;
        constexpr static u32 info = 1;
        constexpr static u32 stages = 2;
    };
}
