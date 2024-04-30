#pragma once

#include "String.hpp"
#include "StringView.hpp"

namespace Fyrion
{
    inline String Last(const StringView& txt, const StringView& separator)
    {
        String ret{};
        Split(txt, separator, [&](const StringView& str)
        {
            ret = str;
        });
        return ret;
    }

    inline String WithoutLast(const StringView& txt, const StringView& separator)
    {
        String     ret{};
        StringView temp{};
        Split(txt, separator, [&](const StringView& str)
        {
            if (!ret.Empty())
            {
                ret += separator;
            }
            ret += temp;
            temp = str;
        });
        return ret;
    }
}
