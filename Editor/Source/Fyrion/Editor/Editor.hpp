#pragma once

#include "Fyrion/Common.hpp"
#include "EditorTypes.hpp"
#include "MenuItem.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Editor/SceneEditor.hpp"

namespace Fyrion::Editor
{
    FY_API void         Init();
    FY_API void         AddMenuItem(const MenuItemCreation& menuItem);
    FY_API void         OpenWindow(TypeID windowType, VoidPtr initUserData = nullptr);
    FY_API void         OpenProject(RID rid);
    FY_API AssetTree&   GetAssetTree();
    FY_API SceneEditor& GetSceneEditor(); //TODO temporary, each workspaces will have one SceneTree.

    template <typename T>
    FY_API void OpenWindow(VoidPtr initUserData = nullptr)
    {
        OpenWindow(GetTypeID<T>(), initUserData);
    }
}
