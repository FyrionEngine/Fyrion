#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "AssetTypes.hpp"


namespace Fyrion
{
    class FY_API AssetDatabase
    {
    public:
        static AssetDirectory* LoadFromDirectory(const StringView& name, const StringView& directory);
        static AssetDirectory* LoadFromFile(const StringView& name, const StringView& file);
        static Asset*          FindById(const UUID& assetId);
        static Asset*          FindByPath(const StringView& path);
        static Asset*          Create(TypeID typeId);
        static Asset*          Create(TypeID typeId, UUID uuid);
        static Asset*          CreateFromPrototype(TypeID typeId, Asset* prototype);
        static Asset*          CreateFromPrototype(TypeID typeId, Asset* prototype, UUID uuid);
        static void            Destroy(Asset* asset, bool destroySubobjects = true);

        template <typename T>
        static T* Create()
        {
            return static_cast<T*>(Create(GetTypeID<T>()));
        }

        template <typename T>
        static T* Create(UUID uuid)
        {
            return static_cast<T*>(Create(GetTypeID<T>(), uuid));
        }

        template <typename T>
        static T* CreateFromPrototype(T* prototype)
        {
            return static_cast<T*>(CreateFromPrototype(GetTypeID<T>(), prototype));
        }

        template <typename T>
        static T* CreateFromPrototype(T* prototype, UUID uuid)
        {
            return static_cast<T*>(CreateFromPrototype(GetTypeID<T>(), prototype, uuid));
        }

        template <typename T>
        static T* FindById(const UUID& assetId)
        {
            return static_cast<T*>(FindById(assetId));
        }
    };
}
