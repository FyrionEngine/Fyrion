#include "SceneTreeWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    MenuItemContext SceneTreeWindow::s_menuItemContext = {};

    void SceneTreeWindow::Draw(u32 id, bool& open)
    {
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(id, ICON_FA_LIST " Scene Tree", &open, ImGuiWindowFlags_NoScrollbar);


        ImGui::End();
    }

    void SceneTreeWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        s_menuItemContext.AddMenuItem(menuItem);
    }

    void SceneTreeWindow::OpenSceneTree(VoidPtr userData)
    {
        Editor::OpenWindow<SceneTreeWindow>();
    }

    void SceneTreeWindow::AddGameObject(VoidPtr userData)
    {
    }

    void SceneTreeWindow::AddGameObjectFromAsset(VoidPtr userData)
    {
    }

    void SceneTreeWindow::AddComponent(VoidPtr userData)
    {
    }

    void SceneTreeWindow::RenameGameObject(VoidPtr userData)
    {
    }

    void SceneTreeWindow::DuplicateGameObject(VoidPtr userData)
    {
    }

    void SceneTreeWindow::DeleteGameObject(VoidPtr userData)
    {
    }

    void SceneTreeWindow::RegisterType(NativeTypeHandler<SceneTreeWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Scene Tree", .action = OpenSceneTree});

        AddMenuItem(MenuItemCreation{.itemName = "Add Game Object", .priority = 0, .action = AddGameObject});
        AddMenuItem(MenuItemCreation{.itemName = "Add Game Object From Asset", .priority = 10, .action = AddGameObjectFromAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Add Component", .priority = 20, .action = AddComponent});
        AddMenuItem(MenuItemCreation{.itemName = "Rename", .priority = 200, .itemShortcut = {.presKey = Key::F2}, .action = RenameGameObject});
        AddMenuItem(MenuItemCreation{.itemName = "Duplicate", .priority = 210, .itemShortcut = {.ctrl = true, .presKey = Key::D}, .action = DuplicateGameObject});
        AddMenuItem(MenuItemCreation{.itemName = "Delete", .priority = 220, .itemShortcut = {.presKey = Key::Delete}, .action = DeleteGameObject});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::TopRight,
            .createOnInit = true
        });
    }

    void InitSceneTreeWindow()
    {
        Registry::Type<SceneTreeWindow, EditorWindow>();
    }
}
