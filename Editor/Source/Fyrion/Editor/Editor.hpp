#pragma once

#include "Fyrion/Common.hpp"
#include "MenuItem.hpp"
#include "Action/EditorAction.hpp"
#include "Asset/AssetEditor.hpp"

namespace Fyrion::Editor
{
    FY_API void               Init(StringView projectFile);
    FY_API void               AddMenuItem(const MenuItemCreation& menuItem);
    FY_API void               OpenWindow(TypeID windowType, VoidPtr initUserData = nullptr);
    FY_API EditorTransaction* CreateTransaction();
    FY_API String             CreateProject(StringView newProjectPath, StringView projectName);


    template <typename T>
    FY_API void OpenWindow(VoidPtr initUserData = nullptr)
    {
        OpenWindow(GetTypeID<T>(), initUserData);
    }
}
