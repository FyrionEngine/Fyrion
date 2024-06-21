#pragma once

#include "Fyrion/Common.hpp"
#include "EditorTypes.hpp"
#include "MenuItem.hpp"
#include "Editor/SceneEditor.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"

namespace Fyrion::Editor
{
    FY_API void                  Init();
    FY_API void                  AddMenuItem(const MenuItemCreation& menuItem);
    FY_API void                  OpenWindow(TypeID windowType, VoidPtr initUserData = nullptr);
    FY_API void                  OpenDirectory(AssetDirectory* directory);
    FY_API Span<AssetDirectory*> GetOpenDirectories();
    FY_API SceneEditor&          GetSceneEditor();

    template <typename T>
    FY_API void OpenWindow(VoidPtr initUserData = nullptr)
    {
        OpenWindow(GetTypeID<T>(), initUserData);
    }
}
