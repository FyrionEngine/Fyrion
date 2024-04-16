#include "WorldViewWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    void WorldViewWindow::Draw(u32 id, bool& open)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        ImGui::Begin(id, ICON_FA_BORDER_ALL " World Viewport", &open, flags);
        ImGui::End();
    }

    void WorldViewWindow::OpenWorldView(VoidPtr userData)
    {
        Editor::OpenWindow<WorldViewWindow>();
    }

    void WorldViewWindow::RegisterType(NativeTypeHandler<WorldViewWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/World View", .action = OpenWorldView});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Center,
            .createOnInit = true
        });
    }

    void InitWorldViewWindow()
    {
        Registry::Type<WorldViewWindow, EditorWindow>();
    }
}
