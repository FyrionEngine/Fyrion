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


    struct GraphNodeValue
    {
        constexpr static u32 Input = 0;
        constexpr static u32 Value = 1;
    };

    struct GraphNodeAsset
    {
        constexpr static u32 NodeType = 0;
        constexpr static u32 Position = 1;
        constexpr static u32 InputValues = 2;
    };

    struct GraphNodeLinkAsset
    {
        constexpr static u32 InputNode = 0;
        constexpr static u32 InputPin = 1;
        constexpr static u32 OutputNode = 2;
        constexpr static u32 OutputPin = 3;
    };
}
