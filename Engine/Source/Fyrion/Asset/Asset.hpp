#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    template <typename T>
    class SubobjectList
    {
    public:
        void Add()
        {
        }

        void Remove()
        {
        }


        void Clear()
        {
        }

    private:
    };


    class Asset
    {
    public:
        virtual ~Asset() = default;

        virtual void Load() = 0;
        virtual void Unload() = 0;

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

    private:
        UUID         uniqueId;
        Asset*       prototype;
        TypeHandler* assetType;
        u64          version;
    };
}
