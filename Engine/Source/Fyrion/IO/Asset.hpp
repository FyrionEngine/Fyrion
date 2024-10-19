#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    struct AssetLoader;

    class FY_API Asset
    {
    public:
        UUID         GetUUID() const;
        TypeHandler* GetTypeHandler() const;
        Array<u8>    LoadStream(usize offset, usize size) const;
        void         SetTypeHandler(TypeHandler* typeHandler);

        friend class Assets;

    private:
        UUID         uuid;
        TypeHandler* typeHandler = nullptr;
        AssetLoader* loader = nullptr;
    };

    struct FY_API AssetLoader
    {
        virtual ~AssetLoader() = default;

        virtual Asset*    LoadAsset() = 0;
        virtual Array<u8> LoadStream(usize offset, usize size) = 0;
    };

    class FY_API Assets
    {
    public:
        static void   Create(UUID uuid, AssetLoader* loader);
        static Asset* Load(UUID uuid);
        static Asset* LoadByPath(StringView path);
        static void   SetPath(UUID uuid, StringView path);

        template <typename T>
        static T* LoadByPath(StringView path)
        {
            return static_cast<T*>(LoadByPath(path));
        }

        template <typename T>
        static T* Load(UUID uuid)
        {
            return static_cast<T*>(Load(uuid));
        }
    };


    struct AssetApi
    {
        Asset* (*CastAsset)(VoidPtr ptr);
        void (*SetAsset)(VoidPtr ptr, Asset* asset);
    };

    template <typename T>
    struct TypeApiInfo<T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>>
    {
        static void ExtractApi(VoidPtr pointer)
        {
            AssetApi* api = static_cast<AssetApi*>(pointer);
            api->CastAsset = [](VoidPtr ptr)
            {
                return static_cast<Asset*>(*static_cast<T**>(ptr));
            };

            api->SetAsset = [](VoidPtr ptr, Asset* asset)
            {
                *static_cast<T**>(ptr) = static_cast<T*>(asset);
            };
        }

        static constexpr TypeID GetApiId()
        {
            return GetTypeID<AssetApi>();
        }
    };
}
