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

    inline String ToUpper(const StringView& string)
    {
        String ret;
        ret.Resize(string.Size());

        for(usize i = 0; i < string.Size(); ++i)
        {
            ret[i] = toupper(string[i]);
        }
        return ret;
    }

    inline bool ContainsIgnoreCase(const StringView& string, const StringView& search)
    {
        String buffer{};
        buffer.Resize(search.Size());
        for (const char* it = string.begin(); it != string.end(); ++it)
        {
            buffer.Clear();

            for (int j = 0; j < search.Size() && it + j != string.end(); ++j)
            {
                buffer[j] = *(it + j);
            }

            if (buffer.Compare(search) == 0)
            {
                return true;
            }
        }
        return false;
    }

    inline String FormatName(const StringView& property)
    {
        String name = property;
        if (!name.Empty())
        {
            name[0] = toupper(name[0]);

            for (int i = 1; i < name.Size(); ++i)
            {
                auto p = name.begin() + i;
                if (*p == *" ")
                {
                    *p = toupper(*p);
                } else if (isupper(*p))
                {
                    bool insertSpace = true;
                    if (i<name.Size()-1)
                    {
                        auto pn = name.begin() + i + 1;
                        if (isupper(*pn))
                        {
                            insertSpace = false;
                        }
                    }

                    if (insertSpace)
                    {
                        name.Insert(p, " ");
                    }
                    i++;
                }
            }
        }
        return name;
    }
}
