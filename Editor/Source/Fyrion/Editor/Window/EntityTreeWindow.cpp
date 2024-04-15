#include "EntityTreeWindow.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/WorldEditor.hpp"
#include "Fyrion/World/World.hpp"


/*
    Create a class WorldEditor to interface with the editor, this will be responsable for all world changes.
    it will sync both World and ResourceObjects.!
*/

namespace Fyrion
{
    MenuItemContext EntityTreeWindow::s_menuItemContext = {};

    class World;

    void EntityTreeWindow::DrawEntity(WorldEditor& worldEditor, EditorEntity* editorEntity)
    {
        if (editorEntity == nullptr) return;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        m_NameCache.Clear();
        m_NameCache += ICON_FA_CUBE;
        m_NameCache += " ";
        m_NameCache += editorEntity->name;

        bool isSelected = editorEntity->selected;
        auto treeFlags = isSelected ? ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAllColumns : ImGuiTreeNodeFlags_SpanAllColumns;

        ImGuiID treeId = 100000 + (ImGuiID) editorEntity->editorId;
        ImGui::TreeLeaf(treeId, m_NameCache.CStr(), treeFlags);

        bool isHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

        if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && isHovered)
        {
            if (!(ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightCtrl))))
            {
                worldEditor.CleanSelection();
            }
            worldEditor.SelectEntity(editorEntity);
        }
    }

    void EntityTreeWindow::Draw(u32 id, bool& open)
    {
        WorldEditor& worldEditor = Editor::GetWorldEditor();

        auto& style = ImGui::GetStyle();
        auto originalWindowPadding = style.WindowPadding;

        bool openPopup = false;

        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(id, ICON_FA_LIST " Entity Tree", &open, ImGuiWindowFlags_NoScrollbar);

        {
            ImGui::StyleVar childWindowPadding(ImGuiStyleVar_WindowPadding, originalWindowPadding);

            ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar;
            ImGui::BeginChild("top-fields", ImVec2(0, (25 * style.ScaleFactor) + originalWindowPadding.y), false, flags);

            if (ImGui::Button(ICON_FA_PLUS))
            {
                openPopup = true;
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

                    if (worldEditor.IsLoaded())
                    {
                        m_NameCache.Clear();
                        m_NameCache += ICON_FA_CUBES;
                        m_NameCache += " ";
                        m_NameCache += worldEditor.GetWorldName();

                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                        bool treeOpen = ImGui::TreeNode(HashValue(worldEditor.GetWorldObject()), m_NameCache.CStr(), ImGuiTreeNodeFlags_SpanAllColumns);

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EntityTreePayload))
                            {
                                // moveEntitiesTo = FY_NULL_ENTITY;
                                // removeSelectionParent = true;
                            }
                            ImGui::EndDragDropTarget();
                        }

                        if (treeOpen)
                        {
                            ImGui::Indent();
                            ImGui::BeginTreeNode();

                            Span<EditorEntity*> entities = worldEditor.GetRootEntities();
                            for(EditorEntity* entity: entities)
                            {
                                DrawEntity(worldEditor, entity);
                            }

                            ImGui::EndTreeNode();
                            ImGui::Unindent();
                            ImGui::TreePop();
                        }
                    }

                    ImGui::EndTable();
                }
            }

            ImGui::EndChild();
        }

        bool closePopup = false;

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            if (s_menuItemContext.ExecuteHotKeys(this))
            {
                closePopup = true;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                openPopup = true;
            }
        }

        if (openPopup)
        {
            ImGui::OpenPopup("entity-tree-popup");
        }

        bool popupRes = ImGui::BeginPopupMenu("entity-tree-popup");
        if (popupRes)
        {
            s_menuItemContext.Draw(this);
            if (closePopup)
            {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopupMenu(popupRes);

        ImGui::End();
    }

    void EntityTreeWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        s_menuItemContext.AddMenuItem(menuItem);
    }

    void EntityTreeWindow::AddEntity(VoidPtr userData)
    {
        Editor::GetWorldEditor().CreateEntity();
    }

    void EntityTreeWindow::AddEntityFromAsset(VoidPtr userData)
    {
    }

    void EntityTreeWindow::AddComponent(VoidPtr userData)
    {
    }

    void EntityTreeWindow::RenameEntity(VoidPtr userData)
    {
    }

    void EntityTreeWindow::DuplicateEntity(VoidPtr userData)
    {
    }

    void EntityTreeWindow::DeleteEntity(VoidPtr userData)
    {

    }

    void EntityTreeWindow::RegisterType(NativeTypeHandler<EntityTreeWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Entity Tree", .action = OpenEntityTree});

        AddMenuItem(MenuItemCreation{.itemName = "Add Entity", .priority = 0, .action = AddEntity});
        AddMenuItem(MenuItemCreation{.itemName = "Add Entity From Asset", .priority = 10, .action = AddEntityFromAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Add Component", .priority = 20, .action = AddComponent});
        AddMenuItem(MenuItemCreation{.itemName = "Rename", .priority = 200, .itemShortcut = {.presKey = Key::F2}, .action = RenameEntity});
        AddMenuItem(MenuItemCreation{.itemName = "Duplicate", .priority = 210, .itemShortcut = {.ctrl = true, .presKey = Key::D}, .action = DuplicateEntity});
        AddMenuItem(MenuItemCreation{.itemName = "Delete", .priority = 220, .itemShortcut = {.presKey = Key::Delete}, .action = DeleteEntity});


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
