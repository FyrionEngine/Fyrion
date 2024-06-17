#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class Asset;

    class FY_API Subobject
    {
    public:
        void Add(Asset* asset);
        void Add(UUID assetId);
        void Remove(UUID uuid);
        void Remove(Asset* asset);

        friend class Asset;

    private:
        Asset* asset = nullptr;
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

        friend class AssetDatabase;

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
