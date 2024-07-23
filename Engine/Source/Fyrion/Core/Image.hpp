#pragma once

#include "Array.hpp"
#include "Fyrion/Common.hpp"
#include "Color.hpp"

namespace Fyrion
{
    template <typename T>
    class FY_API TImage
    {
    public:
        TImage()
        {}

        TImage(Span<const T> buffer);
        TImage(const StringView& file);

        bool Empty() const
        {
            return data.Empty();
        }

        u32 GetWidth() const
        {
            return width;
        }

        u32 GetHeight() const
        {
            return height;
        }

        u32 GetChannels() const
        {
            return channels;
        }

        Span<T> GetData() const
        {
            return data;
        }

        void SetPixelColor(u32 x, u32 y, const TColor<T>& color)
        {
            auto posPixel = (this->channels * (y * this->width + x));
            if (this->channels > 0)
            {
                this->data[posPixel] = color.red;
            }
            if (this->channels > 1)
            {
                this->data[posPixel + 1] = color.green;
            }
            if (this->channels > 2)
            {
                this->data[posPixel + 2] = color.blue;
            }
            if (this->channels > 3)
            {
                this->data[posPixel + 3] = color.alpha;
            }
        }

        [[nodiscard]] TColor<T> GetPixelColor(u32 x, u32 y) const
        {
            auto color = TColor<T>{U8_MAX, U8_MAX, U8_MAX, U8_MAX};
            auto posPixel = this->channels * (y * this->width + x);
            if (this->channels == 1)
            {
                color.alpha = this->data[posPixel];
                return color;
            }
            if (this->channels > 0)
            {
                color.red = this->data[posPixel];
            }
            if (this->channels > 1)
            {
                color.green = this->data[posPixel + 1];
            }
            if (this->channels > 2)
            {
                color.blue = this->data[posPixel + 2];
            }
            if (this->channels > 3)
            {
                color.alpha = this->data[posPixel + 3];
            }
            return color;
        }

    private:
        u32      width = 0;
        u32      height = 0;
        u32      channels = 0;
        Array<T> data;
    };

    using Image = TImage<u8>;
    using HDRImage = TImage<f32>;
}
