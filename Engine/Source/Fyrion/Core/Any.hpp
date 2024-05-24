#pragma once

#include "Registry.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class Any
    {
    public:
        Any(const Any& any)
        {
            if (any.m_typeHandler)
            {
                m_typeHandler = any.m_typeHandler;
                m_data = MemoryGlobals::GetDefaultAllocator().MemAlloc(m_typeHandler->GetTypeInfo().size, m_typeHandler->GetTypeInfo().alignment);
                m_typeHandler->Copy(any.m_data, m_data);
            }
        }

        Any(Any&& any)
        {
            m_data = any.m_data;
            m_typeHandler = any.m_typeHandler;

            any.m_data = nullptr;
            any.m_typeHandler= nullptr;
        }

        Any() = default;

        Any(TypeHandler* typeHandler, VoidPtr data) : m_data(data), m_typeHandler(typeHandler)
        {
        }

        VoidPtr Get() const
        {
            return m_data;
        }

        template <typename T>
        T& GetAs() const
        {
            return *static_cast<T*>(Get());
        }

        void Set(TypeHandler* typeHandler)
        {
            if (m_typeHandler && m_data)
            {
                m_typeHandler->Destroy(m_data);
            }
            m_typeHandler = typeHandler;

            if (m_typeHandler)
            {
                m_data = MemoryGlobals::GetDefaultAllocator().MemAlloc(m_typeHandler->GetTypeInfo().size, m_typeHandler->GetTypeInfo().alignment);
                m_typeHandler->Construct(m_data);
            }

        }

        TypeHandler* GetTypeHandler() const
        {
            return m_typeHandler;
        }

        explicit operator bool() const noexcept
        {
            return this->m_data != nullptr;
        }

        ~Any()
        {
            if (m_typeHandler && m_data)
            {
                m_typeHandler->Destroy(m_data);
            }
        }

    private:
        VoidPtr      m_data{};
        TypeHandler* m_typeHandler{};
    };


    template <typename... Args>
    Any MakeAny(TypeHandler* typeHandler, Args&&... args)
    {
        VoidPtr data = MemoryGlobals::GetDefaultAllocator().MemAlloc(typeHandler->GetTypeInfo().size, typeHandler->GetTypeInfo().alignment);
        typeHandler->Construct(data, Traits::Forward<Args>(args)...);
        return {typeHandler, data};
    }


    template <typename T, typename... Args>
    Any MakeAny(Args&&... args)
    {
        T value{Traits::Forward<Args>(args)...};
        TypeHandler* typeHandler = Registry::FindType<T>();
        VoidPtr data = MemoryGlobals::GetDefaultAllocator().MemAlloc(typeHandler->GetTypeInfo().size, typeHandler->GetTypeInfo().alignment);
        typeHandler->Copy(&value, data);
        return {typeHandler, data};
    }
}
