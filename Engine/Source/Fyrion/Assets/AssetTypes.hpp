#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct UIFont
    {
        constexpr static u32 FontBytes = 0;
    };

    struct TextureAsset
    {
        constexpr static u32 Extent = 0;
        constexpr static u32 Channels = 1;
        constexpr static u32 Data = 2;
    };

    struct ShaderAsset
    {
        constexpr static u32 Bytes = 0;
        constexpr static u32 Info = 1;
        constexpr static u32 Stages = 2;
    };
}
