#include "SceneViewWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{

    void SceneViewWindow::Draw(u32 id, bool& open)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        ImGui::Begin(id, ICON_FA_BORDER_ALL " Scene Viewport", &open, flags);
        ImGui::End();
    }

    void SceneViewWindow::OpenSceneView(VoidPtr userData)
    {
        Editor::OpenWindow<SceneViewWindow>();
    }

    void SceneViewWindow::RegisterType(NativeTypeHandler<SceneViewWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Scene Viewport", .action = OpenSceneView});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Center,
            .createOnInit = true
        });
    }

    void InitSceneViewWindow()
    {
        Registry::Type<SceneViewWindow, EditorWindow>();
    }


}
