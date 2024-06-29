#include "SceneTreeWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    MenuItemContext SceneTreeWindow::s_menuItemContext = {};

    SceneTreeWindow::SceneTreeWindow(): sceneEditor(Editor::GetSceneEditor())
    {
    }

    void SceneTreeWindow::DrawSceneObject(SceneObjectAsset& sceneObject)
    {

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool root = sceneEditor.GetRootObject() == &sceneObject;

        Array<SceneObjectAsset*> children = sceneObject.GetChildren().GetAsArray();

        nameCache.Clear();
        nameCache += root ? ICON_FA_CUBES : ICON_FA_CUBE;
        nameCache += " ";
        nameCache += sceneObject.GetName();

        bool isSelected =  sceneEditor.IsSelected(sceneObject);
        auto treeFlags = isSelected ? ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAllColumns : ImGuiTreeNodeFlags_SpanAllColumns;
        bool open = false;

        ImGuiID treeId = static_cast<ImGuiID>(HashValue(reinterpret_cast<usize>(&sceneObject)));

        if (root)
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        }

        if (sceneEditor.IsParentOfSelected(sceneObject))
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }

        if (isSelected && renamingSelected)
        {
            ImVec2 cursorPos = ImGui::GetCursorPos();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetTreeNodeToLabelSpacing());

            if (!renamingFocus)
            {
                renamingStringCache = sceneObject.GetName();
                ImGui::SetKeyboardFocusHere();
            }

            ImGui::StyleVar framePadding(ImGuiStyleVar_FramePadding, ImVec2{0, 0});

            ImGui::Text(ICON_FA_CUBE);
            ImGui::SameLine();

            auto size = ImGui::CalcTextSize(" ");
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + size.x);

            ImGui::InputText(66554433, renamingStringCache);

            if (!ImGui::IsItemActive() && renamingFocus)
            {
                renamingSelected = false;
                renamingFocus = false;
                sceneEditor.RenameObject(sceneObject, renamingStringCache);
            }

            if (!renamingFocus && renamingSelected)
            {
                renamingFocus = true;
            }

            ImGui::SetCursorPos(cursorPos);

            if (children.Size() > 0)
            {
                open = ImGui::TreeNode(treeId, " ", 0);
            }
        }
        else if (children.Size() > 0)
        {
            open = ImGui::TreeNode(treeId, nameCache.CStr(), treeFlags);
        }
        else
        {
            ImGui::TreeLeaf(treeId, nameCache.CStr(), treeFlags);
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(SceneTreePayload))
            {
                // moveEntitiesTo = FY_NULL_ENTITY;
                // removeSelectionParent = true;
            }
            ImGui::EndDragDropTarget();
        }

        bool isHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

        if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && isHovered)
        {
            if (!(ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightCtrl))))
            {
                sceneEditor.ClearSelection();
            }
            sceneEditor.SelectObject(sceneObject);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && isHovered)
        {
            entityIsSelected = true;
        }

        ImGui::TableNextColumn();
        if (!root)
        {
            ImGui::Text("  " ICON_FA_EYE);
        }

        if (open)
        {
            for (SceneObjectAsset* child : children)
            {
                DrawSceneObject(*child);
            }

            ImGui::TreePop();
        }
    }

    void SceneTreeWindow::Draw(u32 id, bool& open)
    {

        entityIsSelected = false;

        auto& style = ImGui::GetStyle();
        auto originalWindowPadding = style.WindowPadding;

        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(id, ICON_FA_LIST " Scene Tree", &open, ImGuiWindowFlags_NoScrollbar);
        bool openPopup = false;

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
            ImGui::SearchInputText(id + 10, searchEntity);
            ImGui::EndChild();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + originalWindowPadding.y);

        {
            ImGui::StyleVar cellPadding(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
            ImGui::StyleVar frameRounding(ImGuiStyleVar_FrameRounding, 0);
            ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
            ImGui::StyleColor borderColor(ImGuiCol_Border, IM_COL32(45, 46, 48, 255));

            if (ImGui::BeginChild("scene-tree-view-child", ImVec2(0, 0), false))
            {
                static ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody;

                if (ImGui::BeginTable("scene-tree-view-table", 2, tableFlags))
                {
                    ImGui::TableSetupColumn("  Name", ImGuiTableColumnFlags_NoHide);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 35 * style.ScaleFactor);
                    ImGui::TableHeadersRow();

                    if (SceneObjectAsset* root = sceneEditor.GetRootObject())
                    {
                        ImGui::BeginTreeNode();
                        DrawSceneObject(*root);
                        ImGui::EndTreeNode();
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
                if (!entityIsSelected)
                {
                    sceneEditor.ClearSelection();
                    renamingSelected = false;
                }
                openPopup = true;
            }
        }

        if (openPopup)
        {
            ImGui::OpenPopup("scene-tree-popup");
        }

        bool popupRes = ImGui::BeginPopupMenu("scene-tree-popup");
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

    void SceneTreeWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        s_menuItemContext.AddMenuItem(menuItem);
    }

    void SceneTreeWindow::OpenSceneTree(const MenuItemEventData& eventData)
    {
        Editor::OpenWindow<SceneTreeWindow>();
    }

    void SceneTreeWindow::AddSceneObject(const MenuItemEventData& eventData)
    {
        static_cast<SceneTreeWindow*>(eventData.drawData)->sceneEditor.CreateObject();
    }

    void SceneTreeWindow::AddSceneObjectFromAsset(const MenuItemEventData& eventData)
    {
    }

    void SceneTreeWindow::AddComponent(const MenuItemEventData& eventData)
    {
    }

    void SceneTreeWindow::RenameSceneObject(const MenuItemEventData& eventData)
    {
        static_cast<SceneTreeWindow*>(eventData.drawData)->renamingSelected = true;
    }

    void SceneTreeWindow::DuplicateSceneObject(const MenuItemEventData& eventData)
    {
    }

    void SceneTreeWindow::DeleteSceneObject(const MenuItemEventData& eventData)
    {
        static_cast<SceneTreeWindow*>(eventData.drawData)->sceneEditor.DestroySelectedObjects();
    }

    bool SceneTreeWindow::CheckSelectedObject(const MenuItemEventData& eventData)
    {
        return !static_cast<SceneTreeWindow*>(eventData.drawData)->sceneEditor.IsRootSelected();
    }

    void SceneTreeWindow::RegisterType(NativeTypeHandler<SceneTreeWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Scene Tree", .action = OpenSceneTree});

        AddMenuItem(MenuItemCreation{.itemName = "Add Empty Object", .priority = 0, .itemShortcut = {.ctrl = true, .presKey = Key::Space}, .action = AddSceneObject});
        AddMenuItem(MenuItemCreation{.itemName = "Add Object From Asset", .priority = 10, .action = AddSceneObjectFromAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Add Component", .priority = 20, .action = AddComponent});
        AddMenuItem(MenuItemCreation{.itemName = "Rename", .priority = 200, .itemShortcut = {.presKey = Key::F2}, .action = RenameSceneObject, .enable = CheckSelectedObject});
        AddMenuItem(MenuItemCreation{.itemName = "Duplicate", .priority = 210, .itemShortcut = {.ctrl = true, .presKey = Key::D}, .action = DuplicateSceneObject, .enable = CheckSelectedObject});
        AddMenuItem(MenuItemCreation{.itemName = "Delete", .priority = 220, .itemShortcut = {.presKey = Key::Delete}, .action = DeleteSceneObject, .enable = CheckSelectedObject});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::TopRight,
            .createOnInit = true
        });
    }

    void InitSceneTreeWindow()
    {
        Registry::Type<SceneTreeWindow>();
    }
}
