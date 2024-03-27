#pragma once

#include "Fyrion/Common.hpp"
#include "Hash.hpp"
#include "StringConverter.hpp"

namespace Fyrion
{

    template<typename Type>
    class BasePath;

    template<typename Type, usize BufferSize>
    class BasicString;

    template<typename Type>
    class BasicStringView
    {
    public:
        typedef Type ValueType;
        typedef Type* Pointer;
        typedef const Type* ConstPointer;
        typedef ConstPointer Iterator;
        typedef ConstPointer ConstIterator;

        static constexpr usize s_npos = -1;

        constexpr BasicStringView();
        constexpr BasicStringView(const Type* s, usize count);
        constexpr BasicStringView(const Type* s);

        template<usize BufferSize>
        constexpr BasicStringView(const BasicString<Type, BufferSize>& string);

        constexpr BasicStringView(const BasePath<Type>& path);
        constexpr BasicStringView(const BasicStringView&) = default;

        BasicStringView& operator=(const BasicStringView&) = default;

        constexpr const Type* Data() const;
        constexpr ConstPointer CStr() const;

        constexpr char operator[](usize pos) const;

        constexpr usize Size() const;
        constexpr bool Empty() const;

        constexpr Iterator begin() const;
        constexpr ConstIterator cbegin() const;
        constexpr Iterator end() const;
        constexpr ConstIterator cend() const;

        constexpr BasicStringView Substr(usize pos = 0, usize count = s_npos) const;
        constexpr void Swap(BasicStringView& v);
        constexpr i32 Compare(const BasicStringView& other) const;
        constexpr i32 Compare(ConstPointer sz) const;
        static constexpr usize Strlen(const Type*);

        constexpr usize FindFirstOf(Type s, usize pos = 0) const;
        constexpr usize FindFirstOf(const Type* s, usize pos = 0) const;
        constexpr usize FindFirstOf(const Type* s, usize pos, usize n) const;
        constexpr usize FindFirstNotOf(Type s, usize pos = 0) const;
        constexpr usize FindFirstNotOf(const Type* s, usize pos = 0) const;
        constexpr usize FindFirstNotOf(const Type* s, usize pos, usize n) const;
        constexpr usize FindLastOf(Type s, usize pos = s_npos) const;
        constexpr usize FindLastOf(const Type* s, usize pos = s_npos) const;
        constexpr usize FindLastOf(const Type* s, usize pos, usize n) const;
        constexpr usize FindLastNotOf(Type s, usize pos = s_npos) const;
        constexpr usize FindLastNotOf(const Type* s, usize pos = s_npos) const;
        constexpr usize FindLastNotOf(const Type* s, usize pos, usize n) const;

        constexpr bool StartsWith(const BasicStringView<Type>& comp) const;

    private:
        BasicStringView(decltype(nullptr)) = delete;

        constexpr bool IsContained(Type ch, ConstIterator first, ConstIterator last) const;

        const Type* m_str;
        usize m_size;
    };

    template<typename Type>
    constexpr BasicStringView<Type>::BasicStringView(const BasePath<Type>& path) : m_str(path.begin()), m_size(path.end() - path.begin())
    {
    }

    template<typename Type>
    template<usize BufferSize>
    constexpr BasicStringView<Type>::BasicStringView(const BasicString<Type, BufferSize>& string) : m_str(string.CStr()), m_size(string.Size())
    {
    }

    template<typename Type>
    constexpr BasicStringView<Type>::BasicStringView() : m_str(nullptr), m_size(0)
    {
    }

    template<typename Type>
    constexpr BasicStringView<Type>::BasicStringView(const Type* s, usize count) : m_str(s), m_size(count)
    {
    }

    template<typename Type>
    constexpr BasicStringView<Type>::BasicStringView(const Type* s) : m_str(s), m_size(Strlen(s))
    {
    }

    template<typename Type>
    constexpr const Type* BasicStringView<Type>::Data() const
    {
        return m_str;
    }

    template<typename Type>
    constexpr typename BasicStringView<Type>::ConstPointer BasicStringView<Type>::CStr() const
    {
        return m_str;
    }

    template<typename Type>
    constexpr char BasicStringView<Type>::operator[](usize pos) const
    {
        return m_str[pos];
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::Size() const
    {
        return m_size;
    }

    template<typename Type>
    constexpr bool BasicStringView<Type>::Empty() const
    {
        return 0 == m_size;
    }

    template<typename Type>
    constexpr typename BasicStringView<Type>::Iterator BasicStringView<Type>::begin() const
    {
        return m_str;
    }

    template<typename Type>
    constexpr typename BasicStringView<Type>::ConstIterator BasicStringView<Type>::cbegin() const
    {
        return m_str;
    }

    template<typename Type>
    constexpr typename BasicStringView<Type>::Iterator BasicStringView<Type>::end() const
    {
        return m_str + m_size;
    }

    template<typename Type>
    constexpr typename BasicStringView<Type>::ConstIterator BasicStringView<Type>::cend() const
    {
        return m_str + m_size;
    }

    template<typename Type>
    constexpr BasicStringView<Type> BasicStringView<Type>::Substr(usize pos, usize count) const
    {
        return BasicStringView(m_str + pos, s_npos == count ? m_size - pos : count);
    }

    template<typename Type>
    constexpr void BasicStringView<Type>::Swap(BasicStringView<Type>& v)
    {
        Type* strtmp = m_str;
        usize sizetmp = m_size;
        m_str = v.m_str;
        m_size = v.m_size;
        v.m_str = strtmp;
        v.m_size = sizetmp;
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::Strlen(const Type* s)
    {
        for (usize len = 0;; ++len)
        {
            if (0 == s[len])
            {
                return len;
            }
        }
    }

    template<typename Type>
    constexpr i32 BasicStringView<Type>::Compare(const BasicStringView<Type>& other) const
    {
        if (this->Size() != other.Size())
        {
            return false;
        }
        int ctr = 0;
        while (this->m_str[ctr] == other.m_str[ctr])
        {
            if (this->Size() == ctr)
            {
                return 0;
            }
            ctr++;
        }
        if (this->Size() == ctr)
        {
            return 0;
        }
        return -1;
    }

    template<typename Type>
    constexpr i32 BasicStringView<Type>::Compare(typename BasicStringView<Type>::ConstPointer sz) const
    {
        ConstPointer it = CStr();
        if (!it) return -1;
        for (; *it && *sz && (*it == *sz); ++it, ++sz);
        return *it - *sz;
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindFirstOf(const Type* s, usize pos) const
    {
        return FindFirstOf(s, pos, Strlen(s));
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindFirstOf(const Type* s, usize pos, usize n) const
    {
        for (usize i = pos; i != m_size; ++i)
        {
            if (IsContained(m_str[i], s, s + n))
            {
                return i;
            }
        }
        return s_npos;
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindFirstOf(Type s, usize pos) const
    {
        return FindFirstOf(&s, pos, 1);
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindFirstNotOf(Type s, usize pos) const
    {
        return FindFirstNotOf(&s, pos, 1);
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindFirstNotOf(const Type* s, usize pos) const
    {
        return FindFirstNotOf(s, pos, Strlen(s));
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindFirstNotOf(const Type* s, usize pos, usize n) const
    {
        for (usize i = pos; i != m_size; ++i)
        {
            if (!IsContained(m_str[i], s, s + n))
            {
                return i;
            }
        }
        return s_npos;
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindLastOf(Type s, usize pos) const
    {
        return FindLastOf(&s, pos, 1);
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindLastOf(const Type* s, usize pos) const
    {
        return FindLastOf(s, pos, Strlen(s));
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindLastOf(const Type* s, usize pos, usize n) const
    {
        if (pos == s_npos)
        {
            pos = m_size - 1;
        }

        for (usize i = pos; i > 0; --i)
        {
            if (IsContained(m_str[i], s, s + n))
            {
                return i;
            }
        }
        return s_npos;
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindLastNotOf(Type s, usize pos) const
    {
        return 0;
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindLastNotOf(const Type* s, usize pos) const
    {
        if (pos == s_npos)
        {
            pos = m_size - 1;
        }

        return FindLastNotOf(s, pos, Strlen(s));
    }

    template<typename Type>
    constexpr usize BasicStringView<Type>::FindLastNotOf(const Type* s, usize pos, usize n) const
    {
        for (usize i = pos; i >= 0; --i)
        {
            if (!IsContained(m_str[i], s, s + n))
            {
                return i;
            }
        }
        return s_npos;
    }

    template<typename Type>
    constexpr bool BasicStringView<Type>::IsContained(Type ch, BasicStringView::ConstIterator first, BasicStringView::ConstIterator last) const
    {
        for (auto cit = first; cit != last; ++cit)
        {
            if (*cit == ch)
            {
                return true;
            }
        }
        return false;
    }

    template<typename Type>
    constexpr bool BasicStringView<Type>::StartsWith(const BasicStringView<Type>& comp) const
    {
        auto& self = *this;

        if (comp.Size() > self.Size())
        {
            return false;
        }

        for (int i = 0; i < comp.Size(); ++i)
        {
            if (self[i] != comp[i])
            {
                return false;
            }
        }
        return true;
    }

    template<typename Type>
    FY_FINLINE bool operator==(const BasicStringView<Type>& lhs, const BasicStringView<Type>& rhs)
    {
        if (lhs.Size() != rhs.Size())
        {
            return false;
        }

        return lhs.Compare(rhs) == 0;
    }

    template<typename Type>
    FY_FINLINE bool operator==(const BasicStringView<Type>& lhs, typename BasicStringView<Type>::ConstPointer rhs)
    {
        return lhs.Compare(rhs) == 0;
    }

    template<typename Type>
    FY_FINLINE bool operator==(typename BasicStringView<Type>::ConstPointer lhs, const BasicStringView<Type>& rhs)
    {
        return rhs.Compare(lhs) == 0;
    }

    template<typename Type>
    FY_FINLINE bool operator!=(const BasicStringView<Type>& lhs, const BasicStringView<Type>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename Type>
    FY_FINLINE bool operator!=(const BasicStringView<Type>& lhs, typename BasicStringView<Type>::ConstPointer rhs)
    {
        return !(lhs == rhs);
    }

    template<typename Type>
    FY_FINLINE bool operator!=(typename BasicStringView<Type>::ConstPointer lhs, const BasicStringView<Type>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename Type>
    FY_FINLINE bool operator<(const BasicStringView<Type>& lhs, const BasicStringView<Type>& rhs)
    {
        return lhs.Compare(rhs) < 0;
    }

    template<typename Type>
    FY_FINLINE bool operator<(const BasicStringView<Type>& lhs, typename BasicStringView<Type>::ConstPointer rhs)
    {
        return lhs.Compare(rhs) < 0;
    }

    template<typename Type>
    FY_FINLINE bool operator<(typename BasicStringView<Type>::ConstPointer lhs, const BasicStringView<Type>& rhs)
    {
        return rhs.Compare(lhs) > 0;
    }

    template<typename Type>
    FY_FINLINE bool operator>(const BasicStringView<Type>& lhs, const BasicStringView<Type>& rhs)
    {
        return rhs < lhs;
    }

    template<typename Type>
    FY_FINLINE bool operator>(const BasicStringView<Type>& lhs, typename BasicStringView<Type>::ConstPointer rhs)
    {
        return rhs < lhs;
    }

    template<typename Type>
    FY_FINLINE bool operator>(typename BasicStringView<Type>::ConstPointer lhs, const BasicStringView<Type>& rhs)
    {
        return rhs < lhs;
    }

    template<typename Type>
    FY_FINLINE bool operator<=(const BasicStringView<Type>& lhs, const BasicStringView<Type>& rhs)
    {
        return !(rhs < lhs);
    }

    template<typename Type>
    FY_FINLINE bool operator<=(const BasicStringView<Type>& lhs, typename BasicStringView<Type>::ConstPointer rhs)
    {
        return !(rhs < lhs);
    }

    template<typename Type>
    FY_FINLINE bool operator<=(typename BasicStringView<Type>::ConstPointer lhs, const BasicStringView<Type>& rhs)
    {
        return !(rhs < lhs);
    }

    template<typename Type>
    FY_FINLINE bool operator>=(const BasicStringView<Type>& lhs, const BasicStringView<Type>& rhs)
    {
        return !(lhs < rhs);
    }

    template<typename Type>
    FY_FINLINE bool operator>=(const BasicStringView<Type>& lhs, typename BasicStringView<Type>::ConstPointer rhs)
    {
        return !(lhs < rhs);
    }

    template<typename Type>
    FY_FINLINE bool operator>=(typename BasicStringView<Type>::ConstPointer lhs, const BasicStringView<Type>& rhs)
    {
        return !(lhs < rhs);
    }

    using StringView = BasicStringView<char>;

    template<typename Element>
    struct Hash<BasicStringView<Element>>
    {
        constexpr static bool hasHash = true;

        constexpr static usize Value(const BasicStringView<Element>& string)
        {
            usize hash = 0;
            for (auto it = string.begin(); it != string.end(); ++it)
            {
                hash = *it + (hash << 6) + (hash << 16) - hash;
            }
            return hash;
        }
    };

    template<>
    struct StringConverter<BasicStringView<char>>
    {
        typedef char Element;

        constexpr static bool hasConverter = true;
        constexpr static usize bufferCount = 0;

        static usize Size(const BasicStringView<Element>& stringView)
        {
            return stringView.Size();
        }

        static usize ToString(char* buffer, usize pos, const BasicStringView<Element>& value)
        {
            StrCopy(buffer, pos, value.begin(), value.Size());
            return value.Size();
        }

        static void FromString(const char* str, usize size, BasicStringView<Element>& value)
        {

        }
    };
}