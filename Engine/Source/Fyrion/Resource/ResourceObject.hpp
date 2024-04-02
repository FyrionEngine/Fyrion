#pragma once

#include "ResourceTypes.hpp"
#include "Fyrion/Core/Array.hpp"

namespace Fyrion
{
    class TypeHandler;

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
        explicit ResourceObject(ResourceData* data);
        virtual ~ResourceObject();

        ResourceObject(const ResourceObject& object) = delete;
        ResourceObject& operator=(const ResourceObject& object) = delete;

        ConstPtr            GetValue(u32 index) const;
        void                SetValue(u32 index, ConstPtr pointer);
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
        bool                Has(u32 index) const;
        bool                HasNoPrototype(u32 index) const;
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
        void SetValue(u32 index, const T& pointer)
        {
            SetValue(index, &pointer);
        }

        template<typename T, typename = typename Traits::EnableIf<!std::is_pointer_v<T>>>
        const T& GetValue(u32 index)
        {
            return *static_cast<const T*>(GetValue(index));
        }

    private:
        static bool ResourceSubObjectAllowed(u32 index, ResourceData* data, ResourceData* ownerData, const RID& rid);
        static void ResourceGetSubObjectSet(ResourceData* data, ResourceData* ownerData, u32 index, usize& count, Span<RID>* subObjects);

        ResourceData* m_data;
    };

    inline ResourceObjectValue::ResourceObjectValue(u32 index, ResourceObject* resourceObject) : m_index(index), m_resourceObject(resourceObject){}

    template<typename T>
    inline const T& ResourceObjectValue::As()
    {
        return *static_cast<const T*>(m_resourceObject->GetValue(m_index));
    }

    template<typename T>
    inline ResourceObjectValue& ResourceObjectValue::operator=(const T& value)
    {
        m_resourceObject->SetValue(m_index, &value);
        return *this;
    }
}