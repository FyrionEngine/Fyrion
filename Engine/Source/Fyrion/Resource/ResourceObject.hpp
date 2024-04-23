#pragma once

#include "ResourceTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "StreamObject.hpp"

namespace Fyrion
{

    template<typename T>
    struct ObjectValueHandler
    {
        static void SetValue(ResourceObject& resourceObject, u32 index, const T& value);
        static T GetValue(ResourceObject& resourceObject, u32 index);
    };


    class TypeHandler;

    class ResourceObjectValue {
    public:
        ResourceObjectValue(u32 index, ResourceObject* resourceObject);

        template<typename T>
        const T& As();

        template<typename T>
        T Value();

        template<typename T>
        ResourceObjectValue& operator=(const T& value);
    private:
        u32 m_index;
        ResourceObject* m_resourceObject;
    };

    class FY_API ResourceObject
    {
    public:
        explicit ResourceObject(ResourceData* data, bool readPrototypes);
        virtual ~ResourceObject();

        ResourceObject(const ResourceObject& object) = delete;
        ResourceObject& operator=(const ResourceObject& object) = delete;

        ConstPtr            GetValue(u32 index) const;
        void                SetValue(u32 index, ConstPtr pointer);
        VoidPtr             WriteValue(u32 index);
        void                SetSubObject(u32 index, RID subobject);
        RID                 GetSubObject(u32 index);
        void                AddToSubObjectSet(u32 index, RID subObject);
        void                AddToSubObjectSet(u32 index, const Span<RID>& subObjects);
        void                RemoveFromSubObjectSet(u32 index, RID subObject);
        void                RemoveFromSubObjectSet(u32 index, const Span<RID>& subObjects);
        void                ClearSubObjectSet(u32 index);
        usize               GetSubObjectSetCount(u32 index);
        void                GetSubObjectSet(u32 index, Span<RID> subObjects);
        usize               GetRemoveFromPrototypeSubObjectSetCount(u32 index) const;
        void                GetRemoveFromPrototypeSubObjectSet(u32 index, Span<RID> remove) const;
        void                RemoveFromPrototypeSubObjectSet(u32 index, RID remove);
        void                RemoveFromPrototypeSubObjectSet(u32 index, const Span<RID>& remove);
        void                CancelRemoveFromPrototypeSubObjectSet(u32 index, RID remove);
        void                CancelRemoveFromPrototypeSubObjectSet(u32 index, const Span<RID>& remove);
        StreamObject*       GetStream(u32 index);
        StreamObject*       WriteStream(u32 index);
        bool                Has(u32 index) const;
        Array<RID>          GetSubObjectSetAsArray(u32 index);
        u32                 GetValueCount() const;
        u32                 GetIndex(const StringView& name) const;
        StringView          GetName(u32 index) const;
        TypeHandler*        GetFieldType(u32 index) const;
        ResourceFieldType   GetResourceType(u32 index) const;
        RID                 GetRID() const;
        void                Commit();

        explicit operator bool() const;

        template<typename T>
        ResourceObjectValue operator[](T index)
        {
            return {static_cast<u32>(index), this};
        }

        template<typename T, typename = typename Traits::EnableIf<!std::is_pointer_v<T>>>
        void SetValue(u32 index, const T& value)
        {
            ObjectValueHandler<T>::SetValue(*this, index, value);
        }

        template<typename T, typename = typename Traits::EnableIf<!std::is_pointer_v<T>>>
        const T& GetValue(u32 index)
        {
            return *static_cast<const T*>(GetValue(index));
        }

    private:
        static bool ResourceSubObjectAllowed(u32 index, ResourceData* data, ResourceData* ownerData, const RID& rid, bool checkPrototypes);
        static void ResourceGetSubObjectSet(ResourceData* data, ResourceData* ownerData, u32 index, usize& count, Span<RID>* subObjects , bool checkPrototypes);

        ResourceData*   m_data;
        bool            m_readPrototypes;
    };

    inline ResourceObjectValue::ResourceObjectValue(u32 index, ResourceObject* resourceObject) : m_index(index), m_resourceObject(resourceObject){}

    template<typename T>
    const T& ResourceObjectValue::As()
    {
        return *static_cast<const T*>(m_resourceObject->GetValue(m_index));
    }

    template <typename T>
    T ResourceObjectValue::Value()
    {
        return ObjectValueHandler<T>::GetValue(*m_resourceObject, m_index);
    }

    template<typename T>
    ResourceObjectValue& ResourceObjectValue::operator=(const T& value)
    {
        ObjectValueHandler<T>::SetValue(*m_resourceObject, m_index, value);
        return *this;
    }

    template <typename T>
    T ObjectValueHandler<T>::GetValue(ResourceObject& resourceObject, u32 index)
    {
        if (ConstPtr value = resourceObject.GetValue(index))
        {
            return *static_cast<const T*>(resourceObject.GetValue(index));
        }
        return {};
    }

    template<typename T>
    void ObjectValueHandler<T>::SetValue(ResourceObject& resourceObject, u32 index, const T& value)
    {
        resourceObject.SetValue(index, &value);
    }

    template<>
    struct ObjectValueHandler<StringView>
    {
        static void SetValue(ResourceObject& resourceObject, u32 index, const StringView& value)
        {
            String valueStr{value};
            resourceObject.SetValue(index, &valueStr);
        }

        static StringView GetValue(ResourceObject& resourceObject, u32 index)
        {
            if (ConstPtr value = resourceObject.GetValue(index))
            {
                const String& strValue = *static_cast<const String*>(resourceObject.GetValue(index));
                return {strValue};
            }
            return {};
        }
    };

    template<typename T>
    struct ObjectValueHandler<Span<T>>
    {
        static void SetValue(ResourceObject& resourceObject, u32 index, const Span<T>& value)
        {
            FY_ASSERT(false, "TODO");
            // String valueStr{value};
            // resourceObject.SetValue(index, &valueStr);
        }

        static Span<T> GetValue(ResourceObject& resourceObject, u32 index)
        {
            if (ConstPtr value = resourceObject.GetValue(index))
            {
                const Array<T>& arrValue = *static_cast<const Array<T>*>(resourceObject.GetValue(index));
                return {arrValue};
            }
            return {};
        }
    };

    template<usize T>
    struct ObjectValueHandler<char[T]>
    {
        static void SetValue(ResourceObject& resourceObject, u32 index, const StringView& value)
        {
            String valueStr{value};
            resourceObject.SetValue(index, &valueStr);
        }
    };
}
