#pragma once

#include "Fyrion/Common.hpp"
#include "Allocator.hpp"
#include "Hash.hpp"
#include "StringConverter.hpp"
#include "Format.hpp"
#include "Serialization.hpp"


namespace Fyrion
{
    template<typename Type>
    class BasicStringView;

    template<typename T, usize BufferSize>
    class BasicString
    {
    public:
        using Type = T;
        using Pointer = T*;
        using ConstPointer = const T*;
        using Iterator = T*;
        using ConstIterator = const T*;

        BasicString();
        BasicString(usize size);
        BasicString(const BasicString& other);
        BasicString(const BasicStringView<T>& stringView);
        BasicString(BasicString&& other) noexcept;
        BasicString(ConstPointer sz);
        BasicString(ConstPointer first, ConstPointer last);
        BasicString(ConstPointer sz, usize len);
        BasicString(Allocator& allocator);
        BasicString(const BasicStringView<T>& stringView, Allocator& allocator);
        BasicString(ConstPointer sz, Allocator& allocator);
        BasicString(ConstPointer first, ConstPointer last, Allocator& allocator);
        BasicString(ConstPointer sz, usize len, Allocator& allocator);

        ~BasicString();

        Iterator begin();
        Iterator end();
        ConstIterator begin() const;
        ConstIterator end() const;

        BasicString& operator=(const BasicString& other);
        BasicString& operator=(BasicString&& other) noexcept;
        BasicString& operator=(Type ch);
        BasicString& operator=(ConstPointer sz);
        BasicString& operator=(const BasicStringView<T>& stringView);
        BasicString& operator+=(const BasicString& other);
        BasicString& operator+=(Type ch);
        BasicString& operator+=(ConstPointer sz);
        Type operator[](usize pos) const;
        Type& operator[](usize pos);

        ConstPointer CStr() const;
        bool Empty() const;
        usize Size() const;
        usize Capacity() const;
        i32 Compare(const BasicString& other) const;
        i32 Compare(ConstPointer sz) const;
        void Reserve(usize capacity);
        void Resize(usize n);
        void SetSize(usize s);
        void Resize(usize n, Type ch);
        void Clear();
        void Assign(Type ch);
        void Assign(ConstPointer sz);
        void Assign(ConstPointer first, ConstPointer last);
        void Assign(const BasicString& other);
        void PushBack(Type ch);
        BasicString& Append(ConstPointer sz);
        BasicString& Append(Type c);
        BasicString& Append(ConstPointer first, ConstPointer last);
        BasicString& Append(const BasicString& other);
        void Insert(Iterator where, Type ch);
        void Insert(Iterator where, ConstPointer sz);
        void Insert(Iterator where, ConstPointer first, ConstPointer last);
        void Insert(Iterator where, const BasicString& other);
        void Erase(Iterator first, Iterator last);
        void Swap(BasicString& other);

        usize Find(char s) const;

		template<typename Type>
		BasicString& Append(const Type& value)
		{
			static_assert(StringConverter<Type>::hasConverter, "[StringConverter] type has no converter");
			if constexpr (StringConverter<Type>::bufferCount > 0)
			{
				char buffer[StringConverter<Type>::bufferCount];
				auto size = StringConverter<Type>::ToString(buffer, 0, value);
				Append(buffer, buffer + size);
			}
			else
			{
				auto initialSize = Size();
				auto newSize = Size() + StringConverter<Type>::Size(value);
				Resize(newSize);
				StringConverter<Type>::ToString(begin() + initialSize, 0, value);
				m_size = newSize | (m_size & c_longFlag);
			}
			return *this;
		}

    private:
        static constexpr usize c_longFlag = ((usize) 1) << (sizeof(usize) * 8 - 1);

        usize m_size;
        union
        {
            struct
            {
                Pointer m_first;
                Pointer m_capacity;
            };
            Type m_buffer[BufferSize];
        };
        Allocator& m_allocator = MemoryGlobals::GetDefaultAllocator();
    };

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString() : BasicString(MemoryGlobals::GetDefaultAllocator())
    {}

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(const BasicStringView<T>& stringView) : BasicString(stringView, MemoryGlobals::GetDefaultAllocator())
    {}

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(Allocator& allocator) : m_size(0), m_allocator(allocator)
    {
        m_buffer[0] = 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(usize size) : m_size(0)
    {
        Resize(size);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(const BasicString& other) : m_size(0), m_allocator(other.m_allocator)
    {
        Assign(other);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(const BasicStringView<T>& stringView, Allocator& allocator) : m_size(0), m_allocator(allocator)
    {
        Assign(stringView.begin(), stringView.end());
    }

    template<typename T, usize BufferSize>
    BasicString<T, BufferSize>::BasicString(BasicString&& other) noexcept : m_size(0)
    {
        this->~BasicString();

        m_size = other.m_size;
        m_allocator = other.m_allocator;
        if (other.m_size & c_longFlag)
        {
            m_capacity = other.m_capacity;
            m_first = other.m_first;
        }
        else
        {
            for (i32 i = 0; i < other.Size(); ++i)
            {
                m_buffer[i] = other[i];
            }
            m_buffer[other.Size()] = 0;
        }
        other.m_size = 0;
        other.m_first = nullptr;
        other.m_capacity = nullptr;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(ConstPointer sz) : BasicString(sz, MemoryGlobals::GetDefaultAllocator())
    {}

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(ConstPointer first, ConstPointer last) : BasicString(first, last, MemoryGlobals::GetDefaultAllocator())
    {}

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(ConstPointer sz, usize len) : BasicString(sz, len, MemoryGlobals::GetDefaultAllocator())
    {}

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(ConstPointer sz, Allocator& allocator) : m_size(0), m_allocator(allocator)
    {
        Assign(sz);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(ConstPointer first, ConstPointer last, Allocator& allocator) : m_size(0), m_allocator(allocator)
    {
        Assign(first, last);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::BasicString(ConstPointer sz, usize len, Allocator& allocator) : m_size(0), m_allocator(allocator)
    {
        Assign(sz, sz + len);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>::~BasicString()
    {
        if (m_size & c_longFlag)
        {
            m_allocator.MemFree(m_first);
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator=(const BasicString& other)
    {
        if (this != &other)
        {
            Assign(other);
        }
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator=(BasicString&& other) noexcept
    {
        this->~BasicString();

        m_size = other.m_size;
        m_allocator = other.m_allocator;

        if (other.m_size & c_longFlag)
        {
            m_capacity = other.m_capacity;
            m_first = other.m_first;
        }
        else
        {
            for (int i = 0; i < other.Size(); ++i)
            {
                m_buffer[i] = other[i];
            }
            m_buffer[other.Size()] = 0;
        }
        other.m_size = 0;
        other.m_first = nullptr;
        other.m_capacity = nullptr;
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator=(Type ch)
    {
        Assign(ch);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator=(ConstPointer sz)
    {
        Assign(sz);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator=(const BasicStringView<T>& stringView)
    {
        Assign(stringView.begin(), stringView.end());
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator+=(const BasicString& other)
    {
        Append(other);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator+=(Type ch)
    {
        PushBack(ch);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::operator+=(ConstPointer sz)
    {
        Append(sz);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::ConstPointer BasicString<T, BufferSize>::CStr() const
    {
        if (m_size & c_longFlag)
        {
            return m_first;
        }
        else
        {
            return m_buffer;
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool BasicString<T, BufferSize>::Empty() const
    {
        return Size() == 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE usize BasicString<T, BufferSize>::Size() const
    {
        return m_size & ~c_longFlag;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE usize BasicString<T, BufferSize>::Capacity() const
    {
        if (m_size & c_longFlag)
        {
            return m_capacity - m_first - 1;
        }
        else
        {
            return FY_STRING_BUFFER_SIZE - 1;
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::Iterator BasicString<T, BufferSize>::begin()
    {
        if (m_size & c_longFlag)
        {
            return m_first;
        }
        else
        {
            return m_buffer;
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::Iterator BasicString<T, BufferSize>::end()
    {
        if (m_size & c_longFlag)
        {
            return m_first + (m_size & ~c_longFlag);
        }
        else
        {
            return m_buffer + m_size;
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::ConstIterator BasicString<T, BufferSize>::begin() const
    {
        if (m_size & c_longFlag)
        {
            return m_first;
        }
        else
        {
            return m_buffer;
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::ConstIterator BasicString<T, BufferSize>::end() const
    {
        if (m_size & c_longFlag)
        {
            return m_first + (m_size & ~c_longFlag);
        }
        else
        {
            return m_buffer + m_size;
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::Type BasicString<T, BufferSize>::operator[](usize pos) const
    {
        return begin()[pos];
    }

    template<typename T, usize BufferSize>
    FY_FINLINE typename BasicString<T, BufferSize>::Type& BasicString<T, BufferSize>::operator[](usize pos)
    {
        return begin()[pos];
    }

    template<typename T, usize BufferSize>
    FY_FINLINE int BasicString<T, BufferSize>::Compare(const BasicString& other) const
    {
        return Compare(other.CStr());
    }

    template<typename T, usize BufferSize>
    FY_FINLINE int BasicString<T, BufferSize>::Compare(ConstPointer sz) const
    {
        ConstPointer it = CStr();
        for (; *it && *sz && (*it == *sz); ++it, ++sz);
        return *it - *sz;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Reserve(usize cap)
    {
        if (cap <= Capacity())
        {
            return;
        }

        auto newFirst = (Pointer) m_allocator.MemAlloc( cap + 1, alignof(T));
        for (Pointer it = begin(), newIt = newFirst, e = end() + 1; it != e; ++it, ++newIt)
        {
            *newIt = *it;
        }

        if (m_size & c_longFlag)
        {
            m_allocator.MemFree( m_first);
        }
        else
        {
            m_size |= c_longFlag;
        }

        m_first = newFirst;
        m_capacity = m_first + cap + 1;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Resize(usize n)
    {
        Resize(n, 0);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Resize(usize n, Type ch)
    {
        if (Size() < n)
        {
            Reserve(n);
            for (Pointer it = end(), e = begin() + n; it != e; ++it)
            {
                *it = ch;
            }
        }
        Pointer it = begin() + n;
        *it = 0;
        m_size = n | (m_size & c_longFlag);
    }

    template<typename T, usize BufferSize>
    void BasicString<T, BufferSize>::SetSize(usize s)
    {
        m_size = s | (m_size & c_longFlag);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Clear()
    {
        Resize(0);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Assign(Type ch)
    {
        Pointer it = begin();
        *it = ch;
        *(it + 1) = 0;
        m_size = 1 | (m_size & c_longFlag);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Assign(ConstPointer sz)
    {
        if (sz == nullptr) return;

        usize len = 0;
        for (ConstPointer it = sz; *it; ++it)
        {
            ++len;
        }

        Assign(sz, sz + len);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Assign(ConstPointer first, ConstPointer last)
    {
        usize newsize = last - first;
        Reserve(newsize);

        Pointer newit = begin();
        for (ConstPointer it = first; it != last; ++it, ++newit)
        {
            *newit = *it;
        }
        *newit = 0;
        m_size = newsize | (m_size & c_longFlag);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Assign(const BasicString& other)
    {
        Assign(other.begin(), other.end());
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::PushBack(Type ch)
    {
        if (Size() != Capacity())
        {
            Pointer it = end();
            *it = ch;
            *(it + 1) = 0;
            ++m_size;
        }
        else
        {
            Append(&ch, &ch + 1);
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::Append(ConstPointer sz)
    {
        usize len = 0;
        for (ConstPointer it = sz; *it; ++it)
        {
            ++len;
        }
        Append(sz, sz + len);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::Append(ConstPointer first, ConstPointer last)
    {
        const usize newsize = (usize) (Size() + (last - first));
        if (newsize > Capacity())
        {
            Reserve((newsize * 3) / 2);
        }

        Pointer newit = end();
        for (ConstPointer it = first; it != last; ++it, ++newit)
        {
            *newit = *it;
        }

        *newit = 0;
        m_size = newsize | (m_size & c_longFlag);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::Append(const BasicString& other)
    {
        Append(other.begin(), other.end());
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize>& BasicString<T, BufferSize>::Append(T c)
    {
        Append(&c, &c + 1);
        return *this;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Insert(Iterator where, Type ch)
    {
        Insert(where, &ch, &ch + 1);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Insert(Iterator where, ConstPointer sz)
    {
        usize len = 0;
        for (ConstPointer it = sz; *it; ++it)
        {
            ++len;
        }
        Insert(where, sz, sz + len);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Insert(Iterator where, ConstPointer first, ConstPointer last)
    {
        if (first == last)
        {
            return;
        }

        const usize w = where - begin();
        const usize newSize = (usize) (Size() + (last - first));

        if (newSize > Capacity())
        {
            Reserve((newSize * 3) / 2);
        }

        Pointer newIt = begin() + newSize;
        for (Pointer it = end(), e = begin() + w; it >= e; --it, --newIt)
        {
            *newIt = *it;
        }

        newIt = begin() + w;
        for (ConstPointer it = first; it != last; ++it, ++newIt)
        {
            *newIt = *it;
        }

        m_size = newSize | (m_size & c_longFlag);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Insert(Iterator where, const BasicString& other)
    {
        insert(where, other.begin(), other.end());
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Erase(Iterator first, Iterator last)
    {
        if (first == last)
        {
            return;
        }
        const usize newSize = (usize) (Size() + (first - last));
        Pointer newIt = first;
        for (Pointer it = last, e = end(); it != e; ++it, ++newIt)
        {
            *newIt = *it;
        }
        *newIt = 0;
        m_size = newSize | (m_size & c_longFlag);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE usize BasicString<T, BufferSize>::Find(char s) const
    {
        const BasicString<T, BufferSize>& str = *this;

        for (usize i = 0; i < this->Size(); ++i)
        {
            if (str[i] == s)
            {
                return i;
            }
        }

        return nPos;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE void BasicString<T, BufferSize>::Swap(BasicString& other)
    {
        const usize tsize = m_size;
        Pointer tfirst, tcapacity;
        Type tbuffer[FY_STRING_BUFFER_SIZE]{};

        if (tsize & c_longFlag)
        {
            tfirst = m_first;
            tcapacity = m_capacity;
        }
        else
        {
            for (Pointer it = m_buffer, newit = tbuffer, e = m_buffer + tsize + 1; it != e; ++it, ++newit)
            {
                *newit = *it;
            }
        }

        m_size = other.m_size;
        if (other.m_size & c_longFlag)
        {
            m_first = other.m_first;
            m_capacity = other.m_capacity;
        }
        else
        {
            for (Pointer it = other.m_buffer, newit = m_buffer, e = other.m_buffer + m_size + 1; it != e; ++it, ++newit)
            {
                *newit = *it;
            }
        }

        other.m_size = tsize;
        if (tsize & c_longFlag)
        {
            other.m_first = tfirst;
            other.m_capacity = tcapacity;
        }
        else
        {
            for (Pointer it = tbuffer, newit = other.m_buffer, e = tbuffer + tsize + 1; it != e; ++it, ++newit)
            {
                *newit = *it;
            }
        }
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize> operator+(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        BasicString<T, BufferSize> ret;
        ret.Reserve(lhs.Size() + rhs.Size());
        ret += lhs;
        ret += rhs;
        return ret;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize> operator+(const BasicString<T, BufferSize>& lhs, const BasicStringView<T>& rhs)
    {
        BasicString<T, BufferSize> ret;
        ret.Reserve(lhs.Size() + rhs.Size());
        ret += lhs;
        ret += rhs;
        return ret;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize> operator+(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        BasicString ret = lhs;
        ret += rhs;
        return ret;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize> operator+(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        BasicString<T, BufferSize> ret = lhs;
        ret += rhs;
        return ret;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE BasicString<T, BufferSize> operator+(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicStringView<T>& rhs)
    {
        BasicString<T, BufferSize> ret = lhs;
        ret += rhs;
        return ret;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator==(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        if (lhs.Size() != rhs.Size())
        {
            return false;
        }

        return lhs.Compare(rhs) == 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator==(const BasicString<T, BufferSize>& lhs, const BasicStringView<T>& rhs)
    {
        if (lhs.Size() != rhs.Size())
        {
            return false;
        }

        return lhs.Compare(rhs) == 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator==(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        return lhs.Compare(rhs) == 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator==(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        return rhs.Compare(lhs) == 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator!=(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator!=(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator!=(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator!=(const BasicString<T, BufferSize>& lhs, const BasicStringView<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator<(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        return lhs.Compare(rhs) < 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator<(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        return lhs.Compare(rhs) < 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator<(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        return rhs.Compare(lhs) > 0;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator>(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        return rhs < lhs;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator>(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        return rhs < lhs;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator>(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        return rhs < lhs;
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator<=(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        return !(rhs < lhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator<=(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        return !(rhs < lhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator<=(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        return !(rhs < lhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator>=(const BasicString<T, BufferSize>& lhs, const BasicString<T, BufferSize>& rhs)
    {
        return !(lhs < rhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator>=(const BasicString<T, BufferSize>& lhs, typename BasicString<T, BufferSize>::ConstPointer rhs)
    {
        return !(lhs < rhs);
    }

    template<typename T, usize BufferSize>
    FY_FINLINE bool operator>=(typename BasicString<T, BufferSize>::ConstPointer lhs, const BasicString<T, BufferSize>& rhs)
    {
        return !(lhs < rhs);
    }

    template<typename T, usize BufferSize, typename AppendType>
    FY_FINLINE void operator<<(BasicString<T, BufferSize>& lhs, const AppendType& rhs)
    {
        lhs.Append(rhs);
    }

    template<typename Element, usize BufferSize>
    struct Hash<BasicString<Element, BufferSize>>
    {
        constexpr static bool hasHash = true;

        constexpr static usize Value(const BasicStringView<Element>& stringView)
        {
            usize hash = 0;
            for (const char c: stringView)
            {
                hash = c + (hash << 6) + (hash << 16) - hash;
            }
            return hash;
        }

        constexpr static usize Value(const char* ch)
        {
            usize hash = 0;
            for (const char* c = ch; *c != '\0'; ++c)
            {
                hash = *ch + (hash << 6) + (hash << 16) - hash;
            }
            return hash;
        }

        constexpr static usize Value(const BasicString<Element, BufferSize>& string)
        {
            usize hash = 0;
            for (auto it = string.begin(); it != string.end(); ++it)
            {
                hash = *it + (hash << 6) + (hash << 16) - hash;
            }
            return hash;
        }
    };


	template<usize BufferSize>
	struct StringConverter<BasicString<char, BufferSize>>
	{
		constexpr static bool  hasConverter = true;
		constexpr static usize bufferCount  = 0;

		static usize Size(const BasicString<char, BufferSize>& string)
		{
			return string.Size();
		}

		static usize ToString(char* buffer, usize pos, const BasicString<char, BufferSize>& string)
		{
			StrCopy(buffer + pos, string.CStr(), string.Size());
			return string.Size();
		}

		static void FromString(const char* str, usize size, BasicString<char, BufferSize>& string)
		{
			string.Clear();
			string.Resize(size);
			StrCopy(string.begin(), str, size);
		}
	};

    template<usize BufferSize>
    struct ArchiveType<BasicString<char, BufferSize>>
    {
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const BasicString<char, BufferSize>& value)
        {
            writer.WriteString(object, name, value);
        }
    };

    template<usize BufferSize>
    using BufferString = BasicString<char, BufferSize>;
    using String = BasicString<char, FY_STRING_BUFFER_SIZE>;

    template<typename T>
    String ToString(const T& value)
    {
        static_assert(StringConverter<T>::hasConverter);
        usize size = StringConverter<T>::Size(value);
        String string(size);
        StringConverter<T>::ToString(string.begin(), 0, value);
        return string;
    }
}

template<>
struct fmt::formatter<Fyrion::String> : fmt::formatter<std::string_view>
{
    auto format(const Fyrion::String& c, format_context& ctx) const
    {
        return formatter<std::string_view>::format(std::string_view(c.CStr(), c.Size()), ctx);
    }
};