#pragma once

#include "Fyrion/Common.hpp"
#include "EditorTypes.hpp"
#include "MenuItem.hpp"

namespace Fyrion::Editor
{
    FY_API void Init();
    FY_API void AddMenuItem(const MenuItemCreation& menuItem);
    FY_API void OpenWindow(TypeID windowType, VoidPtr initUserData = nullptr);

    template<typename T>
    FY_API void OpenWindow(VoidPtr initUserData = nullptr)
    {
        OpenWindow(GetTypeID<T>(), initUserData);
    }
}