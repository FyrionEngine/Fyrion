#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class Asset;


    struct AssetField
    {
        Asset*        asset = nullptr;
        FieldHandler* field;
        static void   RegisterType(NativeTypeHandler<AssetField>& type);
    };


    class FY_API Subobject : public AssetField
    {
    public:
        FY_BASE_TYPES(AssetField);

        void Add(Asset* asset);
        void Remove(Asset* asset);

        usize Count() const;
        void  Get(Span<Asset*> retAssets) const;

        Array<Asset*> GetAsArray() const
        {
            Array<Asset*> ret(Count());
            Get(ret);
            return ret;
        }

        static void RegisterType(NativeTypeHandler<Subobject>& type);

    private:
        Array<Asset*> assets;
        void  GetTo(Span<Asset*> retAssets, usize pos) const;

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

        static void RegisterType(NativeTypeHandler<Asset>& type);

        friend class AssetDatabase;
        friend class Subobject;

    private:
        UUID         uniqueId{};
        String       path{};
        Asset*       prototype{};
        Subobject*   subobjectOf{};
        TypeHandler* assetType{};
        u64          version{};
        String       name{};
    };
}
