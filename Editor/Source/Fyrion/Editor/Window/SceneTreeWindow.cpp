#include "SceneTreeWindow.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"

namespace Fyrion
{

    void SceneTreeWindow::Draw(u32 id, bool& open)
    {
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(id, ICON_FA_LIST " Scene Tree", &open, ImGuiWindowFlags_NoScrollbar);

        ImGui::End();
    }

    void SceneTreeWindow::RegisterType(NativeTypeHandler<SceneTreeWindow>& type)
    {
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