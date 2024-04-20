#pragma once

#include "Fyrion/Common.hpp"
#include "Traits.hpp"

namespace Fyrion
{

    template<typename Type>
    class BasicStringView;

    template<typename T, usize BufferSize>
    class BasicString;

    template<typename Element, typename Func>
    constexpr void Split(const BasicStringView<Element>& string, const BasicStringView<Element>& delimiter, Func&& func)
    {
        usize start = 0;
        for (int i = 0; i < string.Size(); ++i)
        {
            BasicStringView<Element> comp{string.Data() + i, delimiter.Size()};
            if (comp == delimiter)
            {
                BasicStringView<Element> split{string.Data() + start, i - start};
                if (!split.Empty())
                {
                    func(split);
                }
                start = i + delimiter.Size();
            }
        }
        BasicStringView<Element> split{string.Data() + start, string.Size() - start};
        if (!split.Empty())
        {
            func(split);
        }
    }

    template<typename Type>
    inline void StrCopy(Type* dest, const Type* origin, usize size)
    {
        for (int i = 0; i < size; ++i)
        {
            dest[i] = origin[i];
        }
    }

    template<typename Type>
    inline void StrCopy(Type* dest, usize destPos, const Type* origin, usize size)
    {
        for (int i = 0; i < size; ++i)
        {
            dest[destPos + i] = origin[i];
        }
    }

    constexpr usize Strlen(const char* s)
    {
        for (usize len = 0;; ++len)
        {
            if (0 == s[len])
            {
                return len;
            }
        }
        return 0;
    }

    template<typename T>
    inline constexpr void Swap(T& a, T& b)
    {
        T temp = (T&&) a;
        a = (T&&) b;
        b = (T&&) temp;
    }

    template<typename T, typename F>
    inline T* Partition(T* begin, T* end, const F& comp)
    {
        T* pivot = end - 1;
        T* i = begin - 1;
        for (T* j = begin; j < end - 1; ++j)
        {
            if (comp(*j, *pivot))
            {
                ++i;
                Swap(*i, *j);
            }
        }
        Swap(*(i + 1), *pivot);
        return i + 1;
    }

    template<typename T, typename F>
    inline void Sort(T* begin, T* end, const F& comp)
    {
        if (begin < end)
        {
            T* pivot = Partition(begin, end, comp);
            Sort(begin, pivot, comp);
            Sort(pivot + 1, end, comp);
        }
    }

    template<typename Element>
    inline u64 HexTo64(const BasicStringView<Element>& str)
    {
        u64 res = 0;
        for (const char c: str)
        {
            char v = (c & 0xF) + (c >> 6) | ((c >> 3) & 0x8);
            res = (res << 4) | (u64) v;
        }
        return res;
    }

    FY_FINLINE void MemSet(VoidPtr desc, char value, usize size)
    {
        for (usize i = 0; i < size; ++i)
        {
            *((char*) desc + i) = value;
        }
    }

    FY_FINLINE void MemCopy(VoidPtr dest, ConstPtr src, usize size)
    {
        char* c_src = (char*) src;
        char* c_dest = (char*) dest;

        for (int i = 0; i < size; i++)
        {
            c_dest[i] = c_src[i];
        }
    }

    template<typename Type, typename Func>
    inline void ForEach(Type* begin, Type* end, Func&& func)
    {
        for (Type* it = begin; it != end; ++it)
        {
            func(*it);
        }
    }

    template<typename Type, typename Owner, typename Return, typename ...Args, typename ...Params>
    inline void ForEach(Type* begin, Type* end, Return(Owner::*func)(Args...), Params&& ... args)
    {
        for (Type* it = begin; it != end; ++it)
        {
            ((*it)->*func)(Traits::Forward<Args>(args)...);
        }
    }

    template<typename Type>
    constexpr inline Type Sum(Type* begin, Type* end)
    {
        Type ret{};
        ForEach(begin, end, [&](const Type& value)
        { ret += value; });
        return ret;
    }

    template<typename Type>
    inline Type Subtract(Type value, Type* begin, Type* end)
    {
        ForEach(begin, end, [&](const Type& p)
        { value -= p; });
        return value;
    }

    template<typename T>
    void Insert(T* where, T* first, T* last)
    {
        T* itDest = where;
        for (T* it = first; it != last; ++it, ++itDest)
        {
            *itDest = *it;
        }
    }

    template<class T>
    const T& Max(const T& a, const T& b)
    {
        return (a < b) ? b : a;
    }

    inline usize U64ToHex(u64 value, char* output)
    {
        char buffer[16]{};
        auto base = 16;
        int i = 15;
        do
        {
            auto c = "0123456789abcdef"[value % base];
            buffer[i] = c;
            i--;
            value = value / base;
        } while (value > 0);

        int j = 0;
        while (++i < 16)
        {
            output[j++] = buffer[i];
        }
        output[j] = 0;
        return j;
    }

    template<typename Element>
    usize SearchSubString(BasicStringView<Element> text, BasicStringView<Element> pattern)
    {
        int textLength = text.Size();
        int patternLength = pattern.Size();

        for (int i = 0; i <= textLength - patternLength; ++i)
        {
            int j;

            for (j = 0; j < patternLength; ++j)
            {
                if (text[i + j] != pattern[j])
                {
                    break;
                }
            }

            if (j == patternLength)
            {
                return i;
            }
        }
        return nPos;
    }

    template<typename Element>
    bool Contains(BasicStringView<Element> text, BasicStringView<Element> pattern)
    {
        return SearchSubString(text, pattern) != nPos;
    }

    constexpr usize AppendBytes(usize val, const u8* const first, usize count)
    {
        for (usize i = 0; i < count; ++i)
        {
            val ^= static_cast<usize>(first[i]);
            val *= Prime;
        }
        return val;
    }

    template <typename Type>
    constexpr usize AppendValue(const Type& value)
    {
        return AppendBytes(OffsetBias, &reinterpret_cast<const u8&>(value), sizeof(Type));
    }
}

