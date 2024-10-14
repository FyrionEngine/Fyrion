#pragma once

#include "Span.hpp"
#include "Array.hpp"
#include "Fyrion/Common.hpp"

#include "Serialization.hpp"

namespace Fyrion
{
    class FY_API StreamBuffer
    {
    public:
        Array<u8> Load() const;
        void      Store(Span<u8> data);

        Span<u8> GetInMemoryData() const;

        usize offset = 0;
        usize size = 0;
        Array<u8> inMemoryData;

        //TODO: temporary
        String bufferFile;

        static void RegisterType(NativeTypeHandler<StreamBuffer>& type);
    };
}
