#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Asset;
    struct AssetDirectory;
    struct SubobjectApi;


    struct SubobjectApi
    {
        void (*  SetPrototype)(VoidPtr subobject, VoidPtr prototype);
        usize (* GetOwnedObjectsCount)(VoidPtr subobject);
        void (*  GetOwnedObjects)(VoidPtr subobject, Span<VoidPtr> assets);
        void (*  Remove)(VoidPtr subobject, VoidPtr object);
        TypeID (*GetTypeId)();
    };

    class SubobjectBase
    {
    public:
        virtual              ~SubobjectBase() = default;
        virtual SubobjectApi GetApi() = 0;
    };

    template <typename Type>
    class Subobject : public SubobjectBase
    {
    public:
        void Add(Type* object)
        {
            FY_ASSERT(object, "asset is null");
            if constexpr (Traits::IsBaseOf<Asset, Type>)
            {
                FY_ASSERT(!object->subobjectOf, "asset is already a subobject");
                object->subobjectOf = this;
            }
            objects.EmplaceBack(object);
        }

        void Remove(Type* object)
        {
            FY_ASSERT(object, "asset is null");
            if (Type** it = FindFirst(objects.begin(), objects.end(), object))
            {
                objects.Erase(it);
                object->subobjectOf = nullptr;
            }
        }

        usize Count() const
        {
            usize total = objects.Size();
            if (prototype != nullptr)
            {
                total += prototype->Count();
            }
            return total;
        }

        void Get(Span<Type*> p_objects) const
        {
            GetTo(p_objects, 0);
        }

        Array<Type*> GetAsArray() const
        {
            Array<Type*> ret(Count());
            Get(ret);
            return ret;
        }

        Span<Type*> GetOwnedObjects() const
        {
            return objects;
        }

        SubobjectApi GetApi() override;

        friend class TypeApiInfo<Subobject>;

    private:
        Subobject*   prototype = {};
        Array<Type*> objects;

        void GetTo(Span<Type*> p_objects, usize pos) const
        {
            if (prototype != nullptr)
            {
                prototype->GetTo(p_objects, pos);
            }

            for (Asset* object : objects)
            {
                p_objects[pos++] = object;
            }
        }
    };

    template <typename Type>
    struct TypeApiInfo<Subobject<Type>>
    {
        static void SetPrototypeImpl(VoidPtr subobject, VoidPtr prototype)
        {
            static_cast<Subobject<Type>*>(subobject)->prototype = static_cast<Subobject<Type>*>(prototype);
        }

        static usize GetOwnedObjectsCount(VoidPtr subobject)
        {
            return static_cast<Subobject<Type>*>(subobject)->objects.Size();
        }

        static void GetOwnedObjects(VoidPtr ptr, Span<VoidPtr> retAssets)
        {
            auto subobject = static_cast<Subobject<Type>*>(ptr);

            usize pos = 0;
            for (Type* asset : subobject->objects)
            {
                retAssets[pos++] = asset;
            }
        }

        static void RemoveImpl(VoidPtr subobject, VoidPtr object)
        {
            static_cast<Subobject<Type>*>(subobject)->Remove(static_cast<Type*>(object));
        }

        static TypeID GetTypeIdImpl()
        {
            return GetTypeID<Type>();
        }

        static void ExtractApi(VoidPtr pointer)
        {
            SubobjectApi* api = static_cast<SubobjectApi*>(pointer);
            api->SetPrototype = SetPrototypeImpl;
            api->GetOwnedObjectsCount = GetOwnedObjectsCount;
            api->GetOwnedObjects = GetOwnedObjects;
            api->GetTypeId = GetTypeIdImpl;
            api->Remove = RemoveImpl;
        }

        static constexpr TypeID GetApiId()
        {
            return GetTypeID<SubobjectApi>();
        }
    };

    template <typename Type>
    SubobjectApi Subobject<Type>::GetApi()
    {
        SubobjectApi api{};
        TypeApiInfo<Subobject>::ExtractApi(&api);
        return api;
    }

    template <typename Type>
    class FY_API Value
    {
    public:
        Value& operator=(const Type& pValue)
        {
            hasValue = true;
            value = pValue;
            return *this;
        }

        template <typename Other>
        bool operator==(const Other& other) const
        {
            return Get() == other;
        }

        Type Get() const
        {
            if (hasValue)
            {
                return value;
            }

            if (prototype != nullptr)
            {
                return prototype->Get();
            }

            return {};
        }

        operator Type() const
        {
            return Get();
        }

        explicit operator bool() const
        {
            return hasValue;
        }

        friend class TypeApiInfo<Value>;

    private:
        bool   hasValue = false;
        Type   value = {};
        Value* prototype{};
    };

    struct ValueApi
    {
        void (*SetPrototype)(VoidPtr subobject, VoidPtr prototype);
    };

    template <typename Type>
    struct TypeApiInfo<Value<Type>>
    {
        static void SetPrototypeImpl(VoidPtr value, VoidPtr prototype)
        {
            static_cast<Value<Type>*>(value)->prototype = static_cast<Value<Type>*>(prototype);
        }

        static void ExtractApi(VoidPtr pointer)
        {
            ValueApi* api = static_cast<ValueApi*>(pointer);
            api->SetPrototype = SetPrototypeImpl;
        }

        static constexpr TypeID GetApiId()
        {
            return GetTypeID<ValueApi>();
        }
    };


    class FY_API Asset
    {
    public:
        virtual ~Asset() = default;

        virtual void Load() {}

        virtual void Unload() {}

        UUID GetUniqueId() const
        {
            return uniqueId;
        }

        Asset* GetPrototype() const
        {
            return prototype;
        }

        TypeHandler* GetAssetType() const
        {
            return assetType;
        }

        TypeID GetAssetTypeId() const
        {
            return assetType->GetTypeInfo().typeId;
        }

        StringView GetName() const
        {
            return name;
        }

        StringView GetPath() const
        {
            return path;
        }

        StringView GetAbsolutePath() const
        {
            return absolutePath;
        }

        void SetName(StringView p_name);
        void SetDirectory(Asset* p_directory);

        bool IsActive() const
        {
            return active;
        }

        void SetActive(bool p_active);

        AssetDirectory* GetDirectory() const
        {
            return directory;
        }

        bool IsParentOf(Asset* asset) const;


        friend class AssetDatabase;

        template <typename Type>
        friend class Subobject;

        static void RegisterType(NativeTypeHandler<Asset>& type);

        virtual void BuildPath();
        virtual void OnActiveChanged() {}

    private:
        UUID            uniqueId{};
        String          path{};
        Asset*          prototype{};
        SubobjectBase*  subobjectOf{};
        TypeHandler*    assetType{};
        u64             version{};
        u64             loadedVersion{};
        String          name{};
        String          absolutePath{};
        AssetDirectory* directory{};
        bool            active = true;

        void ValidateName();
    };
}
