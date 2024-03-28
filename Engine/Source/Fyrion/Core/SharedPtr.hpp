#pragma once

#include "Fyrion/Common.hpp"
#include "Traits.hpp"
#include "Allocator.hpp"

namespace Fyrion
{
    template<typename Type>
    class PtrBase
    {
    public:
        using Base = Type;

        PtrBase() : m_instance(nullptr), m_references(nullptr)
        {
        }

        PtrBase(Type* instance, i32* references) : m_instance(instance), m_references(references)
        {
            *m_references = 1;
        }

        virtual ~PtrBase() noexcept
        {
            if (m_references != nullptr)
            {
                *this->m_references = *this->m_references - 1;
                if (*this->m_references <= 0)
                {
                    Allocator& allocator = MemoryGlobals::GetDefaultAllocator();
                    m_instance->~Type();
                    allocator.MemFree(m_instance);
                    allocator.MemFree(m_references);
                }
            }
        }

        i32 RefCount() const
        {
            return *m_references;
        }

    protected:
        template<class Type2>
        void MoveConstructorFrom(PtrBase<Type2>&& right) noexcept
        {
            this->m_instance   = right.m_instance;
            this->m_references = right.m_references;

            right.m_references = nullptr;
            right.m_instance   = nullptr;
        }

        template<class Type2>
        void CopyConstructorFrom(const PtrBase<Type2>& right)
        {
            this->m_instance   = right.m_instance;
            this->m_references = right.m_references;
            if (this->m_references != nullptr)
            {
                *this->m_references = *this->m_references + 1;
            }
        }

        template<class Type2>
        void AliasConstructFrom(const PtrBase<Type2>& right, Type* p_instance)
        {
            this->m_instance   = p_instance;
            this->m_references = right.m_references;
            if (this->m_references != nullptr)
            {
                *this->m_references = *this->m_references + 1;
            }
        }

        template<class Type2, typename Alloc2>
        void AliasConstructFrom(PtrBase<Type2>&& right, Type* p_instance)
        {
            this->m_instance   = p_instance;
            this->m_references = right.m_references;

            right.m_references = nullptr;
            right.m_instance   = nullptr;
        }

    private:
        template<class Type0>
        friend
        class PtrBase;

        template<class Type0>
        friend
        class SharedPtr;

        Type* m_instance;
        i32 * m_references;
    };

    template<typename Type>
    class SharedPtr : public PtrBase<Type>
    {
    public:

        SharedPtr() : PtrBase<Type>()
        {}

        SharedPtr(Traits::NullPtr) : PtrBase<Type>()
        {}

        explicit SharedPtr(Type* instance) : PtrBase<Type>(instance, MemoryGlobals::GetDefaultAllocator().Alloc<i32>())
        {}

        SharedPtr(Type* instance, TypeID typeId) : PtrBase<Type>(instance, MemoryGlobals::GetDefaultAllocator().Alloc<i32>(), typeId)
        {}

        SharedPtr(const SharedPtr& sharedPtr)
        {
            this->CopyConstructorFrom(sharedPtr);
        }

        SharedPtr(SharedPtr&& sharedPtr) noexcept
        {
            this->MoveConstructorFrom(Traits::Move(sharedPtr));
        }

        template<typename Type2>
        SharedPtr(const SharedPtr<Type2>& sharedPtr)
        {
            this->CopyConstructorFrom(sharedPtr);
        }

        template<typename Type2>
        SharedPtr(SharedPtr<Type2>&& sharedPtr) noexcept
        {
            this->MoveConstructorFrom(Traits::Move(sharedPtr));
        }

        template<typename Type2>
        SharedPtr(const SharedPtr<Type2>& sharedPtr, Type* ptr)
        {
            this->AliasConstructFrom(sharedPtr, ptr);
        }

        template<typename Type2>
        SharedPtr(SharedPtr<Type2>&& sharedPtr, Type* ptr) noexcept
        {
            this->AliasConstructFrom(Traits::Move(sharedPtr), ptr);
        }

        SharedPtr& operator=(const SharedPtr& sharedPtr) noexcept
        {
            this->CopyConstructorFrom(sharedPtr);
            return *this;
        }

        template<typename Type2>
        SharedPtr& operator=(const SharedPtr<Type2>& sharedPtr) noexcept
        {
            this->CopyConstructorFrom(sharedPtr);
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

        Type* Get() const noexcept
        {
            return this->m_instance;
        }

        Type& operator*() const noexcept
        {
            return *Get();
        }

        Type* operator->() const noexcept
        {
            return Get();
        }

        explicit operator bool() const noexcept
        {
            return this->m_instance != nullptr;
        }
    };

    template<typename Type, typename ...Args>
    inline SharedPtr<Type> MakeShared(Args&& ... args)
    {
        return SharedPtr<Type>(MemoryGlobals::GetDefaultAllocator().Alloc<Type>(Traits::Forward<Args>(args)...));
    }
//
//    template<typename Type1, typename Type2>
//    inline SharedPtr<Type1> staticPointerCast(const SharedPtr<Type2>& sharedPtr) {
//        const auto ptr = static_cast<typename SharedPtr<Type1>::Base*>(sharedPtr.get());
//        return SharedPtr<Type1>(sharedPtr, ptr);
//    }
//
//    template<typename Type1, typename Type2>
//    inline SharedPtr<Type1> staticPointerCast(SharedPtr<Type2>&& sharedPtr) {
//        const auto ptr = static_cast<typename SharedPtr<Type1>::Base*>(sharedPtr.get());
//        return SharedPtr<Type1>(Traits::Move(sharedPtr), ptr);
//    }
}

