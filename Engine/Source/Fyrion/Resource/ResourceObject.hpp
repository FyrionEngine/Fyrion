#pragma once

#include "ResourceTypes.hpp"
#include "Fyrion/Core/Array.hpp"

namespace Fyrion
{
    class ResourceObjectValue {
    public:
        ResourceObjectValue(u32 index, ResourceObject* resourceObject);

        template<typename T>
        const T& As();

        template<typename T>
        ResourceObjectValue& operator=(const T& value);
    private:
        u32 m_index;
        ResourceObject* m_resourceObject;
    };

    class FY_API ResourceObject
    {
    public:
        explicit ResourceObject(ResourceStorage* storage);

        ResourceObject(const ResourceObject&) = delete;
        ResourceObject& operator=(const ResourceObject&) = delete;
        virtual ~ResourceObject();

        ConstPtr    GetValue(u32 index);
        void        SetValue(u32 index, ConstPtr pointer);
        void        Commit();
        void        Rollback();

        template<typename T>
        ResourceObjectValue operator[](T index)
        {
            return {static_cast<u32>(index), this};
        }

    private:
        ResourceStorage*   m_storage{};
        VoidPtr            m_data{};
        Array<VoidPtr>     m_fields{};
        VoidPtr            m_dataOnWrite{};
    };

    ResourceObjectValue::ResourceObjectValue(u32 index, ResourceObject* resourceObject) : m_index(index), m_resourceObject(resourceObject){}

//    template<typename T>
//    ResourceObjectValue::operator T()
//    {
//        //TODO find a better way of casting.
//        return *static_cast<const T*>(m_resourceObject->GetValue(m_index));
//    }

    template<typename T>
    const T& ResourceObjectValue::As()
    {
        return *static_cast<const T*>(m_resourceObject->GetValue(m_index));
    }

    template<typename T>
    ResourceObjectValue& ResourceObjectValue::operator=(const T& value)
    {
        m_resourceObject->SetValue(m_index, &value);
        return *this;
    }
}