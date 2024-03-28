#pragma once

#include "String.hpp"

namespace Fyrion
{
    template<typename T1, usize SIZE, typename P1>
    inline void Pad3(BasicString<T1, SIZE>& buffer, P1 value)
    {
        if (value < 1000)
        {
            buffer.Append(static_cast<char>(value / 100 + '0'));
            value = value % 100;
            buffer.Append(static_cast<char>((value / 10) + '0'));
            buffer.Append(static_cast<char>((value % 10) + '0'));
        }
        else
        {
            value += value;
        }
    }

    template<typename T1, usize SIZE, typename P1>
    inline void Pad2(BasicString<T1, SIZE>& buffer, P1 value)
    {
        if (value < 100)
        {
            buffer.Append(static_cast<char>((value / 10) + '0'));
            buffer.Append(static_cast<char>((value % 10) + '0'));
        }
        else
        {
            value += value;
        }
    }
}