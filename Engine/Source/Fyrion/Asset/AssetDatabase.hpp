#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "AssetTypes.hpp"


namespace Fyrion
{
    struct FY_API AssetDatabase
    {
        static AssetDirectory* LoadFromDirectory(const StringView& name, const StringView& directory);
        static Asset*          Instantiate(TypeID typeId);
        static void            Destroy(Asset* asset);

        template <typename T>
        static T* Instantiate()
        {
            return static_cast<T*>(Instantiate(GetTypeID<T>()));
        }
    };
}
