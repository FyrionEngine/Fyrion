#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "AssetTypes.hpp"


namespace Fyrion
{
    struct FY_API AssetDatabase
    {
        static AssetDirectory* LoadFromDirectory(const StringView& name, const StringView& directory);
        static Asset*          FindById(const UUID& assetId);
        static Asset*          FindByPath(const StringView& path);
        static Asset*          Create(TypeID typeId);
        static Asset*          CreateFromPrototype(TypeID typeId, Asset* prototype);
        static void            Destroy(Asset* asset);

        template <typename T>
        static T* Create()
        {
            return static_cast<T*>(Create(GetTypeID<T>()));
        }

        template <typename T>
        static T* CreateFromPrototype(T* prototype)
        {
            return static_cast<T*>(CreateFromPrototype(GetTypeID<T>(), prototype));
        }
    };
}
