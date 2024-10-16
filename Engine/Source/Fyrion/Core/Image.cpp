#include "Image.hpp"
#include <stb_image.h>
#include <stb_image_resize.h>

#include "Span.hpp"


namespace Fyrion
{
    template<> TImage<u8>::TImage(Span<const u8> buffer)
    {
        i32 imageWidth{};
        i32 imageHeight{};
        i32 imageChannels{};

        auto bytes = stbi_load_from_memory(buffer.Data(), buffer.Size(), &imageWidth, &imageHeight, &imageChannels, 0);

        width = imageWidth;
        height = imageHeight;
        channels = imageChannels;

        data.Resize(width * height * channels);
        MemCopy(data.begin(), bytes, data.Size());

        stbi_image_free(bytes);

        if (channels == 3)
        {
            AddChannel(255);
        }
    }

    template<> TImage<u8>::TImage(const StringView& file)
    {
        i32 imageWidth{};
        i32 imageHeight{};
        i32 imageChannels{};

        u8* bytes = stbi_load(file.CStr(), &imageWidth, &imageHeight, &imageChannels, 0);

        width = imageWidth;
        height = imageHeight;
        channels = imageChannels;

        data.Resize(width * height * channels);
        MemCopy(data.begin(), bytes, data.Size());

        stbi_image_free(bytes);

        if (channels == 3)
        {
            AddChannel(255);
        }
    }

    template<> void TImage<u8>::Resize(u32 width, u32 height)
    {
        Array<u8> newData;
        newData.Resize(width * height * channels);

        stbir_resize_uint8(data.Data(), this->width, this->height, 0,
            newData.begin(),
            width,
            height,
            0,
            GetChannels());

        this->width = width;
        this->height = height;
        data = newData;
    }

    template<> TImage<f32>::TImage(const StringView& file)
    {
        i32 imageWidth{};
        i32 imageHeight{};
        i32 imageChannels{};

        f32* bytes = stbi_loadf(file.CStr(), &imageWidth, &imageHeight, &imageChannels, 0);

        width = imageWidth;
        height = imageHeight;
        channels = imageChannels;

        data.Resize(width * height * channels);
        MemCopy(data.begin(), bytes, data.Size() * sizeof(float));

        stbi_image_free(bytes);

        if (channels == 3)
        {
            AddChannel(255);
        }
    }


    template<> void TImage<f32>::Resize(u32 width, u32 height)
    {

    }
}
