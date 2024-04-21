#include "ProjectBrowserWindow.hpp"

#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Scene/SceneAssets.hpp"

#define CONTENT_TABLE_ID 500
#define ASSET_PAYLOAD "ASSET-PAYLOAD"

namespace Fyrion
{
    MenuItemContext ProjectBrowserWindow::s_menuItemContext = {};

    ProjectBrowserWindow::ProjectBrowserWindow() : m_assetTree(Editor::GetAssetTree())
    {
    }

    void ProjectBrowserWindow::Init(u32 id, VoidPtr userData)
    {
        m_windowId = id;
        if (!m_assetTree.GetAssetRoots().Empty())
        {
            SetOpenFolder(m_assetTree.GetAssetRoots().Back());
        }
    }

    void ProjectBrowserWindow::SetOpenFolder(RID folder)
    {
        m_openFolder = folder;
        m_selectedItem = {};
        AssetNode* node = m_assetTree.GetNode(folder);
        if (node != nullptr && node->parent != nullptr)
        {
            m_openTreeFolders[node->parent->rid] = true;
        }
    }

    void ProjectBrowserWindow::DrawTreeNode(AssetNode* node)
    {
        if (!node->active) return;

        RID rid = node->rid;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

        if (m_openTreeFolders[rid])
        {
            ImGui::SetNextItemOpen(true);
        }

        if (m_openFolder == rid)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        m_stringCache.Clear();
        if (m_openTreeFolders[rid])
        {
            m_stringCache = ICON_FA_FOLDER_OPEN;
        }
        else
        {
            m_stringCache = ICON_FA_FOLDER;
        }
        m_stringCache += " " + node->name;
        bool isOpenNode = ImGui::TreeNode(HashValue(rid), m_stringCache.CStr(), flags);


        if (ImGui::BeginDragDropTarget())
        {
            bool accept = !m_assetTree.IsParentOf(rid, m_movingItem) && m_movingItem != rid;

            if (accept && ImGui::AcceptDragDropPayload(ASSET_PAYLOAD))
            {
                m_assetTree.Move(rid, m_movingItem);
            }
            ImGui::EndDragDropTarget();
        }

        if (node->root != node->rid && ImGui::BeginDragDropSource())
        {
            m_movingItem = rid;
            ImGui::SetDragDropPayload(ASSET_PAYLOAD, nullptr, 0);
            ImGui::Text("%s", node->name.CStr());
            ImGui::EndDragDropSource();
        }

        if ((m_openTreeFolders[rid] == isOpenNode) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            SetOpenFolder(node->rid);
        }

        m_openTreeFolders[rid] = isOpenNode;

        if (isOpenNode)
        {
            for (AssetNode* child: node->nodes)
            {
                if (child->type == GetTypeID<AssetDirectory>())
                {
                    DrawTreeNode(child);
                }
            }
            ImGui::TreePop();
        }
    }

    void ProjectBrowserWindow::DrawPathItems()
    {
        m_folderCache.Clear();

        AssetNode* openFolderNode = m_assetTree.GetNode(m_openFolder);
        RID nextFolder = {};

        {
            AssetNode* drawItem = openFolderNode;
            while (drawItem != nullptr)
            {
                m_folderCache.EmplaceBack(drawItem);
                drawItem = drawItem->parent;
            }
        }

        for (usize i = m_folderCache.Size(); i > 0; --i)
        {
            AssetNode* drawItem = m_folderCache[i - 1];

            String name = drawItem->name;
            if (ImGui::Button(name.CStr()))
            {
                nextFolder = drawItem->rid;
            }

            if (i > 1)
            {
                bool openPopup = false;
                ImGui::PushID((i32) Hash<RID>::Value(drawItem->rid));
                if (ImGui::Button(ICON_FA_ARROW_RIGHT))
                {
                    openPopup = true;
                }
                ImGui::PopID();

                if (openPopup)
                {
                    m_popupFolder = drawItem->rid;
                    ImGui::OpenPopup("select-folder-browser-popup");
                }
            }
        }

        if (nextFolder)
        {
            SetOpenFolder(nextFolder);
        }

        auto popupRes = ImGui::BeginPopupMenu("select-folder-browser-popup");
        if (popupRes && m_popupFolder)
        {
            for(AssetNode* node : m_assetTree.GetNode(m_popupFolder)->nodes)
            {
                if (node->type == GetTypeID<AssetDirectory>())
                {
                    if (ImGui::MenuItem(node->name.CStr()))
                    {
                        SetOpenFolder(node->rid);
                    }
                }
            }
        }
        ImGui::EndPopupMenu(popupRes);
    }

    void ProjectBrowserWindow::Draw(u32 id, bool& open)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 pad = style.WindowPadding;

        AssetNode* assetNodeTree = m_assetTree.GetNode(m_openFolder);
        bool readOnly = assetNodeTree == nullptr;

        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::StyleVar cellPadding(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

        ImGui::StyleColor tableBorderStyleColor(ImGuiCol_TableBorderLight, IM_COL32(0, 0, 0, 0));

        ImGui::Begin(id, ICON_FA_FOLDER " Project Browser", &open, ImGuiWindowFlags_NoScrollbar);

        //top child
        {
            ImVec2 a = ImVec2(pad.x / 1.5f, pad.y / 1.5f);
            ImGui::StyleVar childPadding(ImGuiStyleVar_WindowPadding, a);
            f32 width = ImGui::GetContentRegionAvail().x - a.x;
            ImGui::BeginChild(id + 5, ImVec2(width, 30 * style.ScaleFactor), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);

            ImGui::BeginHorizontal((i32) id + 10, ImVec2(width - a.x - pad.x, 20 * style.ScaleFactor));

            ImGui::BeginDisabled(readOnly);
            if (ImGui::Button(ICON_FA_PLUS " Import"))
            {
                Array<String> paths{};

                if (Platform::OpenDialogMultiple(paths, {}, {}) == DialogResult::OK)
                {
                    if (!paths.Empty())
                    {
                        for (const String& path: paths)
                        {
                            ResourceAssets::ImportAsset(assetNodeTree->root, assetNodeTree->rid, path);
                        }
                        m_assetTree.MarkDirty();
                    }
                }
            }
            ImGui::EndDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

            DrawPathItems();

            ImGui::Spring(1.0f);

            ImGui::PopStyleColor(2);

            ImGui::SetNextItemWidth(400 * style.ScaleFactor);
            ImGui::SearchInputText(id + 20, m_searchString);


            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

            if (ImGui::Button(ICON_FA_GEAR " Settings"))
            {
            }

            ImGui::PopStyleColor(2);
            ImGui::EndHorizontal();

            ImGui::EndChild();

        }

        auto* drawList = ImGui::GetWindowDrawList();

        auto p1 = ImGui::GetCursorScreenPos();
        auto p2 = ImVec2(ImGui::GetContentRegionAvail().x + p1.x, p1.y);
        drawList->AddLine(p1, p2, IM_COL32(0, 0, 0, 255), 1.f * style.ScaleFactor);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.f * style.ScaleFactor);

        bool browseFolder = true;
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("table-project-browser", browseFolder ? 2 : 1, flags))
        {
            ImGui::TableSetupColumn("one", ImGuiTableColumnFlags_WidthFixed, 300 * style.ScaleFactor);
            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
                ImGui::StyleVar rounding(ImGuiStyleVar_FrameRounding, 0);
                ImGui::BeginChild(52110);

                ImGui::BeginTreeNode();

                for(AssetNode* node : m_assetTree.GetRootNodes())
                {
                    DrawTreeNode(node);
                }

                ImGui::EndTreeNode();
                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(27, 28, 30, 255));
                auto padding = 5.f * style.ScaleFactor;
                ImGui::StyleVar cellPadding(ImGuiStyleVar_CellPadding, ImVec2(padding, padding));
                ImGui::StyleVar browserWinPadding(ImGuiStyleVar_WindowPadding, ImVec2(padding * 2, padding));

                ImGui::BeginChild(52211, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
                ImGui::SetWindowFontScale(0.9);

                RID selectedFolder = {};
                if (ImGui::BeginContentTable(id + CONTENT_TABLE_ID, 96 * ImGui::GetStyle().ScaleFactor))
                {
                    AssetNode* openFolderNode = m_assetTree.GetNode(m_openFolder);

                    if (m_openFolder && openFolderNode)
                    {
                        Span<AssetNode*> nodes = openFolderNode->nodes;
                        for (AssetNode* node: nodes)
                        {
                            if (!node->active) continue;

                            if (node->type == GetTypeID<AssetDirectory>())
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId        = Hash<RID>::Value(node->rid);
                                contentItem.ShowDetails   = false;
                                contentItem.Label         = node->name.CStr();
                                contentItem.SetPayload    = ASSET_PAYLOAD;
                                contentItem.AcceptPayload = ASSET_PAYLOAD;
                                contentItem.TooltipText = node->path.CStr();
                                //contentItem.Texture = folderTexture;

                                if (node->updated)
                                {
                                    contentItem.PreLabel = "*";
                                }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    ImGui::ContentItemSelected(U32_MAX);
                                    selectedFolder = node->rid;
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    m_selectedItem = node->rid;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    m_assetTree.Rename(node->rid, ImGui::ContentRenameString());
                                }

                                if (ImGui::ContentItemAcceptPayload(contentItem.ItemId))
                                {
                                    m_assetTree.Move(node->rid, m_movingItem);
                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    m_movingItem = m_selectedItem;
                                }
                            }
                        }

                        for (AssetNode* node: nodes)
                        {
                            if (!node->active) continue;

                            if (node->type == GetTypeID<Asset>())
                            {
                                ResourceObject assetObject = Repository::Read(node->rid);

                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId      = Hash<RID>::Value(node->rid);
                                contentItem.ShowDetails = true;
                                contentItem.Label       = node->name.CStr();
                                contentItem.DetailsDesc = node->assetDesc.CStr();
                                contentItem.SetPayload  = ASSET_PAYLOAD;
                                contentItem.TooltipText = node->path.CStr();
                                //contentItem.Texture = folderTexture;

                                if (node->updated)
                                {
                                    contentItem.PreLabel = "*";
                                }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    if (node->objectType == GetTypeID<SceneObjectAsset>())
                                    {
                                        Editor::GetSceneEditor().LoadScene(node->rid);
                                    }
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    m_selectedItem = node->rid;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    m_assetTree.Rename(node->rid, ImGui::ContentRenameString());
                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    m_movingItem = m_selectedItem;
                                }
                            }
                        }
                    }
                    ImGui::EndContentTable();

                    if (!ImGui::RenamingSelected(CONTENT_TABLE_ID + id) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
                    {
                        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && m_openFolder)
                        {
                            ResourceObject directory = Repository::Read(m_openFolder);
                            if (directory.Has(AssetDirectory::Parent))
                            {
                                selectedFolder = directory[AssetDirectory::Parent].As<RID>();
                            }
                        }
                    }
                }

                if (selectedFolder)
                {
                    SetOpenFolder(selectedFolder);
                }

                ImGui::SetWindowFontScale(1.0);
                ImGui::EndChild();
            }
            ImGui::EndTable();
        }

        bool closePopup = false;
        if (!ImGui::RenamingSelected(CONTENT_TABLE_ID + id) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            if (s_menuItemContext.ExecuteHotKeys(this))
            {
                closePopup = true;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                ImGui::OpenPopup("project-browser-popup");
            }
        }

        auto popupRes = ImGui::BeginPopupMenu("project-browser-popup");
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

    void ProjectBrowserWindow::AssetNewFolder(VoidPtr userData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(userData);
        RID newDirectory = projectBrowserWindow->m_assetTree.NewDirectory(projectBrowserWindow->m_openFolder, "New Folder");

        ImGui::SelectContentItem(Hash<RID>::Value(newDirectory), CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetNewScene(VoidPtr userData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(userData);
        RID newAsset = projectBrowserWindow->m_assetTree.NewAsset(projectBrowserWindow->m_openFolder, Repository::CreateResource<SceneObjectAsset>(), "New Scene");

        ImGui::SelectContentItem(Hash<RID>::Value(newAsset), CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetDelete(VoidPtr userData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(userData);
        projectBrowserWindow->m_assetTree.Delete(projectBrowserWindow->m_selectedItem);
        projectBrowserWindow->m_selectedItem = {};
    }

    bool ProjectBrowserWindow::CheckSelectedAsset(VoidPtr userData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(userData);
        return projectBrowserWindow->m_selectedItem != RID{} && Repository::IsAlive(projectBrowserWindow->m_selectedItem);
    }

    void ProjectBrowserWindow::AssetRename(VoidPtr userData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(userData);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetShowInExplorer(VoidPtr userData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(userData);
        RID itemToShow = projectBrowserWindow->m_selectedItem ? projectBrowserWindow->m_selectedItem : projectBrowserWindow->m_openFolder;
        StringView path = ResourceAssets::GetAbsolutePath(itemToShow);
        if (!path.Empty())
        {
            Platform::ShowInExplorer(path);
        }
    }

    void ProjectBrowserWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        s_menuItemContext.AddMenuItem(menuItem);
    }

    void ProjectBrowserWindow::Shutdown()
    {
        s_menuItemContext = MenuItemContext{};
    }

    void ProjectBrowserWindow::RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Project Browser", .action = OpenProjectBrowser});
        Event::Bind<OnShutdown, Shutdown>();

        AddMenuItem(MenuItemCreation{.itemName="New Folder", .icon=ICON_FA_FOLDER, .priority = 0, .action = AssetNewFolder});
        AddMenuItem(MenuItemCreation{.itemName="New Scene", .icon=ICON_FA_GLOBE, .priority = 10, .action = AssetNewScene});
        AddMenuItem(MenuItemCreation{.itemName="Delete", .icon=ICON_FA_TRASH, .priority = 20, .itemShortcut {.presKey = Key::Delete}, .action = AssetDelete, .enable= CheckSelectedAsset});
        AddMenuItem(MenuItemCreation{.itemName="Rename", .icon=ICON_FA_PEN_TO_SQUARE, .priority = 30, .itemShortcut {.presKey = Key::F2}, .action = AssetRename, .enable= CheckSelectedAsset});
        AddMenuItem(MenuItemCreation{.itemName="Show in Explorer", .icon=ICON_FA_FOLDER, .priority = 40, .action = AssetShowInExplorer});

        type.Function<&ProjectBrowserWindow::SetOpenFolder>("SetOpenFolder");
        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Bottom,
            .createOnInit = true
        });
    }

    void ProjectBrowserWindow::OpenProjectBrowser(VoidPtr userData)
    {
        Editor::OpenWindow(GetTypeID<ProjectBrowserWindow>());
    }

    void InitProjectBrowser()
    {
        Registry::Type<ProjectBrowserWindow, EditorWindow>();
    }


}