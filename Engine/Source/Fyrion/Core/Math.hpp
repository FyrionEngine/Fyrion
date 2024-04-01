#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct Extent
    {
        u32 width;
        u32 height;
    };

    struct Vec4
    {
        union
        {
            struct
            {
                union
                {
                    Float x, r, width;
                };

                union
                {
                    Float y, g, height;
                };

                union
                {
                    Float z, b;
                };

                union
                {
                    Float w, a;
                };

            };
            Float c[4] = {0};
        };
    };

    inline bool operator==(const Vec4& l, const Vec4& r)
    {
        return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
    }

    struct Rect
    {
        i32 x;
        i32 y;
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