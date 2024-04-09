#include "EntityTreeWindow.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/Editor/Editor.hpp"

namespace Fyrion
{

    void EntityTreeWindow::Draw(u32 id, bool& open)
    {
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(id, ICON_FA_LIST " Entity Tree", &open, ImGuiWindowFlags_NoScrollbar);

        ImGui::End();
    }

    void EntityTreeWindow::RegisterType(NativeTypeHandler<EntityTreeWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Entity Tree", .action = EntityTreeWindow::OpenEntityTree});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::TopRight,
            .createOnInit = true
        });
    }

    void EntityTreeWindow::OpenEntityTree(VoidPtr userData)
    {
        Editor::OpenWindow<EntityTreeWindow>();
    }

    void InitSceneTreeWindow()
    {
        Registry::Type<EntityTreeWindow, EditorWindow>();
    }
}