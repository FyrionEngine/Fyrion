#pragma once
#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct WorldAsset
    {
        constexpr static u32 Entities = 0;
    };

    struct EntityAsset
    {
        constexpr static u32 Name = 0;
        constexpr static u32 Components = 1;
        constexpr static u32 Parent = 2;
        constexpr static u32 Children = 3;
    };
}
