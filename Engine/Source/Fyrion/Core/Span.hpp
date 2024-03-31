#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    template<typename T>
    class Array;

    template<typename T, usize bufferSize>
    class FixedArray;

    template<typename T>
    class Span
    {
    public:
        typedef T ValueType;
        typedef T* Iterator;
        typedef const T* ConstIterator;

        Span() : m_first(0), m_last(0)
        {}

        Span(Array<T>& vec) : m_first(vec.Data()), m_last(vec.Data() + vec.Size())
        {
        }

        Span(const Array<T>& vec) : m_first((T*) vec.Data()), m_last((T*) vec.Data() + vec.Size())
        {
        }

        template<usize size>
        Span(const FixedArray<T, size>& vec) : m_first((T*) vec.Data()), m_last((T*) vec.Data() + vec.Size())
        {
        }

        Span(T* t) : m_first(t), m_last(t + 1)
        {
        }

        Span(T* first, T* last) : m_first(first), m_last(last)
        {}

        Span(T* first, usize size) : m_first(first), m_last(first + size)
        {}

//        Span(const std::initializer_list<T>& initializerList) : m_first(initializerList.begin()), m_last(initializerList.end())
//        {
//        };

        Span(std::initializer_list<T> initializerList) : m_first((T*) initializerList.begin()), m_last((T*) initializerList.end())
        {

        };

        constexpr const T* Data() const
        {
            return begin();
        }

        constexpr Iterator begin()
        {
            return m_first;
        }

        constexpr Iterator end()
        {
            return m_last;
        }

        constexpr ConstIterator begin() const
        {
            return m_first;
        }

        constexpr ConstIterator end() const
        {
            return m_last;
        }

        constexpr usize Size() const
        {
            return (usize) (end() - begin());
        }

        constexpr bool Empty() const
        {
            return Size() == 0;
        }

        constexpr T& operator[](usize idx)
        {
            return begin()[idx];
        }

        constexpr const T& operator[](usize idx) const
        {
            return begin()[idx];
        }

        constexpr const T& Back() const
        {
            return begin()[Size() - 1];
        }

        constexpr T& Back()
        {
            return begin()[Size() - 1];
        }

    private:
        T* m_first;
        T* m_last;
    };

    template<typename T>
    bool operator==(const Span<T>& l, const Array<T>& r)
    {
        if (l.Size() != r.Size()) return false;

        for (usize i = 0; i < l.Size(); ++i)
        {
            if (l[i] != r[i])
            {
                return false;
            }
        }
        return true;
    }

}