#pragma once

#include "Span.hpp"
#include "Array.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class FY_API StreamBuffer
    {
    public:
        Array<u8> Load();
        void      Store(Span<u8> data);
    private:
    };
}
