#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion::AssetManager
{

    FY_API void LoadPackage(StringView name, StringView path);

    template<typename T>
    T* LoadByPath(StringView path)
    {
        return nullptr;
    }
}
