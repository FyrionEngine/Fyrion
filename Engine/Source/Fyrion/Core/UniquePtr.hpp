#pragma once

#include "Allocator.hpp"

namespace Fyrion
{

    template<typename T>
    class UniquePtr
    {
    public:
        UniquePtr() : m_instance(nullptr), m_allocator(MemoryGlobals::GetDefaultAllocator())
        {}

        explicit UniquePtr(Traits::NullPtr) : m_instance(nullptr), m_allocator(MemoryGlobals::GetDefaultAllocator())
        {}

        explicit UniquePtr(T* value) : m_instance(value), m_allocator(MemoryGlobals::GetDefaultAllocator())
        {}

        UniquePtr(T* value, Allocator& allocator) : m_instance(value), m_allocator(allocator)
        {
        }

        virtual ~UniquePtr()
        {
            if (m_instance)
            {
                m_allocator.DestroyAndFree(m_instance);
            }
        }

        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr& ptr) = delete;

        UniquePtr(UniquePtr&& other) noexcept : m_allocator(other.m_allocator)
        {
            this->Swap(other);
        }

        UniquePtr& operator=(UniquePtr&& other) noexcept
        {
            other.Swap(*this);
            return *this;
        }

        bool operator==(Traits::NullPtr) const noexcept
        {
            return this->m_instance == nullptr;
        }

        bool operator!=(Traits::NullPtr) noexcept
        {
            return this->m_instance != nullptr;
        }

        T* Get() const noexcept
        {
            return m_instance;
        }

        T& operator*() const noexcept
        {
            return *Get();
        }

        T* operator->() const noexcept
        {
            return Get();
        }

        void Swap(UniquePtr& right) noexcept
        {
            m_instance = right.m_instance;
            m_allocator = right.m_allocator;
            right.m_instance = nullptr;
        }

        explicit operator bool() const noexcept
        {
            return this->m_instance != nullptr;
        }

    private:
        T* m_instance;
        Allocator& m_allocator;
    };


    template<typename Type, typename ...Args>
    inline UniquePtr<Type> MakeUnique(Args&& ... args)
    {
        return UniquePtr<Type>(MemoryGlobals::GetDefaultAllocator().Alloc<Type>(Traits::Forward<Args>(args)...));
    }

}