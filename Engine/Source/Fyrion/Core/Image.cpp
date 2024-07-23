#include "Image.hpp"
#include <stb_image.h>

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
    }
}
