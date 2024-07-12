#include "Image.hpp"

#include <stb_image.h>


namespace Fyrion
{
    TImage<u8>::TImage(const StringView& file)
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
    }
}
