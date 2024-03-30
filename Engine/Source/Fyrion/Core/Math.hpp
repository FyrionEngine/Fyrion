#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct Extent
    {
        u32 width;
        u32 height;
    };



    namespace Math
    {
        template<typename T>
        constexpr auto Min(T a, T b) -> T
        {
            return a < b ? a : b;
        }

        template<typename T>
        constexpr auto Max(T a, T b) -> T
        {
            return a > b ? a : b;
        }

        template<typename T>
        constexpr auto Clamp(T x, T min, T max) -> T
        {
            if (x < min)
            {
                return min;
            }
            if (x > max)
            {
                return max;
            }
            return x;
        }
    }
}