#include "EntityTreeWindow.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/Editor/Editor.hpp"

namespace Fyrion
{
    void EntityTreeWindow::DrawEntity(Entity entity)
    {

    }

    void EntityTreeWindow::Draw(u32 id, bool& open)
    {
        auto& style = ImGui::GetStyle();
        auto originalWindowPadding = style.WindowPadding;

        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(id, ICON_FA_LIST " Entity Tree", &open, ImGuiWindowFlags_NoScrollbar);

        {
            ImGui::StyleVar childWindowPadding(ImGuiStyleVar_WindowPadding, originalWindowPadding);

            ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar;
            ImGui::BeginChild("top-fields", ImVec2(0, (25 * style.ScaleFactor) + originalWindowPadding.y), false, flags);

            if (ImGui::Button(ICON_FA_PLUS))
            {
                ImGui::OpenPopup("entity-tree-popup");
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            ImGui::SearchInputText(id + 10, m_searchEntity);
            ImGui::EndChild();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + originalWindowPadding.y);

        {
            ImGui::StyleVar cellPadding(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
            ImGui::StyleVar frameRounding(ImGuiStyleVar_FrameRounding, 0);
            ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
            ImGui::StyleColor borderColor(ImGuiCol_Border, IM_COL32(45, 46, 48, 255));

            if (ImGui::BeginChild("entity-tree-view-child", ImVec2(0, 0), false))
            {
                static ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody;

                if (ImGui::BeginTable("entity-tree-view-table", 2, tableFlags))
                {
                    ImGui::TableSetupColumn("  Name", ImGuiTableColumnFlags_NoHide);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 35 * style.ScaleFactor);
                    ImGui::TableHeadersRow();

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::EndTable();
                }

                ImGui::EndChild();
            }
        }

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
