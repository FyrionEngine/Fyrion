#pragma once
#include "Math.hpp"


namespace Fyrion
{
    template <typename T>
    struct TColor
    {
        T red;
        T green;
        T blue;
        T alpha;

        f32 FloatRed() const
        {
            return static_cast<f32>(red) / 255;
        }

        f32 FloatGreen() const
        {
            return static_cast<f32>(green) / 255;
        }

        f32 FloatBlue() const
        {
            return static_cast<f32>(blue) / 255;
        }

        f32 FloatAlfa() const
        {
            return static_cast<f32>(alpha) / 255;
        }

        bool operator==(TColor& other)
        {
            return this->red == other.red && this->green == other.green && this->blue == other.blue && this->alpha == other.alpha;
        }

        bool operator!=(TColor& other)
        {
            return !(*this == other);
        }

        auto ToVec4() const
        {
            return Vec4{FloatRed(), FloatGreen(), FloatBlue(), FloatAlfa()};
        }

        auto ToVec3() const
        {
            return Vec3{FloatRed(), FloatGreen(), FloatBlue()};
        }

        static auto FromVec4(const Vec4& color)
        {
            return TColor{static_cast<u8>(color.x * 255), static_cast<u8>(color.y * 255), static_cast<u8>(color.z * 255), static_cast<u8>(color.w * 255)};
        }

        static auto FromVec4(const f32* vec4)
        {
            return TColor{static_cast<u8>(vec4[0] * 255), static_cast<u8>(vec4[1] * 255), static_cast<u8>(vec4[2] * 255), static_cast<u8>(vec4[3] * 255)};
        }

        static auto FromVec3(const f32* vec4)
        {
            return TColor{static_cast<u8>(vec4[0] * 255), static_cast<u8>(vec4[1] * 255), static_cast<u8>(vec4[2] * 255), static_cast<u8>(255)};
        }

        static void FromVec4(TColor& color, const Vec4& vecColor)
        {
            color = TColor{static_cast<u8>(vecColor.x * 255), static_cast<u8>(vecColor.y * 255), static_cast<u8>(vecColor.z * 255), static_cast<u8>(vecColor.w * 255)};
        }

        static TColor FromU32(u32 in)
        {
            return TColor{
                ((in >> FY_COL32_R_SHIFT) & 0xFF),
                ((in >> FY_COL32_G_SHIFT) & 0xFF),
                ((in >> FY_COL32_B_SHIFT) & 0xFF),
                ((in >> FY_COL32_A_SHIFT) & 0xFF)
            };
        }

        static TColor RED;
        static TColor GREEN;
        static TColor YELLOW;
        static TColor BLACK;
        static TColor BLUE;
        static TColor WHITE;
        static TColor CORNFLOWER_BLUE;
        static TColor TRANSPARENT_BLACK;
        static TColor TRANSPARENT_WHITE;
        static TColor NORMAL;

        static void RegisterType(NativeTypeHandler<TColor>& type);
    };

    using Color = TColor<u8>;

    template<> inline Color Color::RED = {255, 0, 0, 255};
    template<> inline Color Color::GREEN = {0, 255, 0, 255};
    template<> inline Color Color::YELLOW = {255, 255, 0, 255};
    template<> inline Color Color::BLACK = {0, 0, 0, 255};
    template<> inline Color Color::WHITE = {255, 255, 255, 255};
    template<> inline Color Color::TRANSPARENT_WHITE = {255, 255, 255, 0};
    template<> inline Color Color::BLUE = {0, 0, 255, 255};
    template<> inline Color Color::CORNFLOWER_BLUE = {100, 149, 237, 255};
    template<> inline Color Color::TRANSPARENT_BLACK = {0, 0, 0, 0};
    template<> inline Color Color::NORMAL = {127, 127, 255, 255};

}
