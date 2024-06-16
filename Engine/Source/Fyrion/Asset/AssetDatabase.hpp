#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "AssetTypes.hpp"


namespace Fyrion::AssetDatabase
{
    FY_API AssetDirectory* LoadFromDirectory(const StringView& name, const StringView& directory);
    FY_API VoidPtr Instantiate(TypeID typeId);


    template<typename T>
    FY_API T* Instantiate()
    {
        return static_cast<T*>(Instantiate(GetTypeID<T>()));
    }

}
