#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"
#include <algorithm>


namespace Fyrion
{
    class Asset;
    class AssetDirectory;

    template <typename Type>
    class FY_API Subobject
    {
    public:
        void Add(Type* asset)
        {
            FY_ASSERT(asset, "asset is null");
            FY_ASSERT(!asset->subobjectOf, "asset is already a subobject");

            asset->subobjectOf = this;
            assets.EmplaceBack(asset);
        }

        void Remove(Type* asset)
        {
            FY_ASSERT(asset, "asset is null");
            if (const auto it = std::find(assets.begin(), assets.end(), asset); it != assets.end())
            {
                assets.Erase(it);
            }
        }

        usize Count() const
        {
            usize total = assets.Size();
            if (prototype != nullptr)
            {
                total += prototype->Count();
            }
            return total;
        }

        void Get(Span<Type*> retAssets) const
        {
            GetTo(retAssets, 0);
        }

        Array<Type*> GetAsArray() const
        {
            Array<Type*> ret(Count());
            Get(ret);
            return ret;
        }

        friend class TypeApiInfo<Subobject>;

    private:
        Subobject*   prototype = {};
        Array<Type*> assets;

        void GetTo(Span<Type*> retAssets, usize pos) const
        {
            if (prototype != nullptr)
            {
                prototype->GetTo(retAssets, pos);
            }

            for (Asset* asset : assets)
            {
                retAssets[pos++] = asset;
            }
        }
    };

    struct SubobjectApi
    {
        void (* SetPrototype)(VoidPtr subobject, VoidPtr prototype);
        usize (*GetOwnedObjectsCount)(VoidPtr subobject);
        void (* GetOwnedObjects)(VoidPtr subobject, Span<Asset*> assets);
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
            return static_cast<Subobject<Type>*>(subobject)->assets.Size();
        }

        static void GetOwnedObjects(VoidPtr ptr, Span<Asset*> retAssets)
        {
            auto subobject = static_cast<Subobject<Type>*>(ptr);

            usize pos = 0;
            for (Asset* asset : subobject->assets)
            {
                retAssets[pos++] = asset;
            }
        }

        static void ExtractApi(VoidPtr pointer)
        {
            SubobjectApi* api = static_cast<SubobjectApi*>(pointer);
            api->SetPrototype = SetPrototypeImpl;
            api->GetOwnedObjectsCount = GetOwnedObjectsCount;
            api->GetOwnedObjects = GetOwnedObjects;
        }

        static constexpr TypeID GetApiId()
        {
            return GetTypeID<SubobjectApi>();
        }
    };

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

        virtual void Load()
        {
        }

        virtual void Unload()
        {
        }

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

        StringView GetName() const
        {
            return name;
        }

        StringView GetPath() const
        {
            return path;
        }

        void SetPath(StringView p_path);
        void SetName(StringView p_name);

        static void RegisterType(NativeTypeHandler<Asset>& type);

        friend class AssetDatabase;

        template <typename Type>
        friend class Subobject;

    private:
        UUID            uniqueId{};
        String          path{};
        Asset*          prototype{};
        VoidPtr         subobjectOf{};
        TypeHandler*    assetType{};
        u64             version{};
        u64             loadedVersion{};
        String          name{};
        String          absolutePath{};
        AssetDirectory* directory{};

        void SetDirectory(AssetDirectory* p_directory);
        void BuildPath();
    };
}
