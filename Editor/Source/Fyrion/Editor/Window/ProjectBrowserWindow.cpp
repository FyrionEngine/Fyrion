#include "ProjectBrowserWindow.hpp"

#include "GraphEditorWindow.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Editor/Action/AssetEditorActions.hpp"

#define CONTENT_TABLE_ID 500
#define ASSET_PAYLOAD "ASSET-PAYLOAD"

namespace Fyrion
{
    MenuItemContext ProjectBrowserWindow::s_menuItemContext = {};

    void ProjectBrowserWindow::Init(u32 id, VoidPtr userData)
    {
        m_windowId = id;
        if (!Editor::GetOpenDirectories().Empty())
        {
            SetOpenDirectory(Editor::GetOpenDirectories().Back());
        }
    }

    void ProjectBrowserWindow::SetOpenDirectory(AssetDirectory* p_directory)
    {
        m_openDirectory = p_directory;
        m_selectedItem = {};

        if (p_directory->GetDirectory() != nullptr)
        {
            m_openTreeFolders[p_directory->GetDirectory()->GetUniqueId()] = true;
        }
    }

    void ProjectBrowserWindow::DrawTreeNode(Asset* asset)
    {
        if (asset == nullptr) return;

        AssetDirectory* directory = asset->GetAssetType()->Cast<AssetDirectory>(asset);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

        bool openDir = m_openTreeFolders[asset->GetUniqueId()];

        if (openDir)
        {
            ImGui::SetNextItemOpen(true);
        }

        if (m_openDirectory == asset)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        m_stringCache.Clear();
        if (openDir)
        {
            m_stringCache = ICON_FA_FOLDER_OPEN;
        }
        else
        {
            m_stringCache = ICON_FA_FOLDER;
        }

        m_stringCache.Append(" ").Append(asset->GetName());

        bool isNodeOpen = ImGui::TreeNode(HashValue(reinterpret_cast<usize>(asset)), m_stringCache.CStr(), flags);

        if (ImGui::BeginDragDropTarget())
        {
            if (m_movingItem != nullptr && !asset->IsParentOf(m_movingItem) && ImGui::AcceptDragDropPayload(ASSET_PAYLOAD))
            {
                //m_movingItem->Move(directory, Editor::CreateTransaction());
            }
            ImGui::EndDragDropTarget();
        }

        if (asset->GetDirectory() != nullptr && ImGui::BeginDragDropSource())
        {
            m_movingItem = asset;
            ImGui::SetDragDropPayload(ASSET_PAYLOAD, nullptr, 0);
            ImGui::Text("%s", asset->GetName().CStr());
            ImGui::EndDragDropSource();
        }

        if (openDir == isNodeOpen && ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            SetOpenDirectory(directory);
        }

        m_openTreeFolders[directory->GetUniqueId()] = isNodeOpen;

        if (isNodeOpen)
        {
            for (Asset* child : directory->children.GetOwnedObjects())
            {
                if (AssetDirectory* childDirectory = child->GetAssetType()->Cast<AssetDirectory>(child))
                {
                    DrawTreeNode(childDirectory);
                }
            }
            ImGui::TreePop();
        }
    }

    void ProjectBrowserWindow::DrawPathItems()
    {
#if 0
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
#endif
    }

    void ProjectBrowserWindow::Draw(u32 id, bool& open)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2      pad = style.WindowPadding;

        bool readOnly = false;

        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::StyleVar cellPadding(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

        ImGui::StyleColor tableBorderStyleColor(ImGuiCol_TableBorderLight, IM_COL32(0, 0, 0, 0));

        ImGui::Begin(id, ICON_FA_FOLDER " Project Browser", &open, ImGuiWindowFlags_NoScrollbar);

        //top child
        {
            ImVec2          a = ImVec2(pad.x / 1.5f, pad.y / 1.5f);
            ImGui::StyleVar childPadding(ImGuiStyleVar_WindowPadding, a);
            f32             width = ImGui::GetContentRegionAvail().x - a.x;
            ImGui::BeginChild(id + 5, ImVec2(width, 30 * style.ScaleFactor), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);

            ImGui::BeginHorizontal((i32)id + 10, ImVec2(width - a.x - pad.x, 20 * style.ScaleFactor));

            ImGui::BeginDisabled(readOnly);
            if (ImGui::Button(ICON_FA_PLUS " Import"))
            {
                Array<String> paths{};

                if (Platform::OpenDialogMultiple(paths, {}, {}) == DialogResult::OK)
                {
                    if (!paths.Empty())
                    {
                        for (const String& path : paths)
                        {
                            AssetDatabase::ImportAsset(m_openDirectory, path);
                        }
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

        bool                   browseFolder = true;
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("table-project-browser", browseFolder ? 2 : 1, flags))
        {
            ImGui::TableSetupColumn("one", ImGuiTableColumnFlags_WidthFixed, 300 * style.ScaleFactor);
            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
                ImGui::StyleVar   rounding(ImGuiStyleVar_FrameRounding, 0);
                ImGui::BeginChild(52110);

                ImGui::BeginTreeNode();

                for (AssetDirectory* directory : Editor::GetOpenDirectories())
                {
                    DrawTreeNode(directory);
                }

                ImGui::EndTreeNode();
                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(27, 28, 30, 255));
                auto              padding = 5.f * style.ScaleFactor;
                ImGui::StyleVar   cellPadding(ImGuiStyleVar_CellPadding, ImVec2(padding, padding));
                ImGui::StyleVar   browserWinPadding(ImGuiStyleVar_WindowPadding, ImVec2(padding * 2, padding));

                ImGui::BeginChild(52211, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SetWindowFontScale(m_contentBrowserZoom);

                AssetDirectory* selectedDiretory = nullptr;

                if (ImGui::BeginContentTable(id + CONTENT_TABLE_ID, m_contentBrowserZoom * 112 * ImGui::GetStyle().ScaleFactor))
                {
                    if (m_openDirectory)
                    {
                        for (Asset* asset : m_openDirectory->children.GetOwnedObjects())
                        {
                            if (!asset->IsAlive()) continue;

                            if (asset->GetAssetTypeId() == GetTypeID<AssetDirectory>())
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId = reinterpret_cast<usize>(asset);
                                contentItem.ShowDetails = false;
                                contentItem.Label = asset->GetName().CStr();
                                contentItem.SetPayload = ASSET_PAYLOAD;
                                contentItem.AcceptPayload = ASSET_PAYLOAD;
                                contentItem.TooltipText = asset->GetPath().CStr();
                                //contentItem.Texture = folderTexture;

                                // if (asset->updated)
                                // {
                                //     contentItem.PreLabel = "*";
                                // }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    ImGui::ContentItemSelected(U32_MAX);
                                    selectedDiretory = asset->GetAssetType()->Cast<AssetDirectory>(asset);
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    m_selectedItem = asset;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    Editor::CreateTransaction()->CreateAction<RenameAssetAction>(asset, ImGui::ContentRenameString())->Commit();
                                }

                                if (ImGui::ContentItemAcceptPayload(contentItem.ItemId))
                                {
                                   // asset->Move(m_movingItem, Editor::CreateTransaction());
                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    m_movingItem = m_selectedItem;
                                }
                            }
                        }

                        for (Asset* asset : m_openDirectory->children.GetOwnedObjects())
                        {
                            if (!asset->IsAlive()) continue;

                            if (asset->GetAssetTypeId() != GetTypeID<AssetDirectory>())
                            {
                                //ResourceObject assetObject = Repository::Read(node->rid);

                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId = reinterpret_cast<usize>(asset);
                                contentItem.ShowDetails = true;
                                contentItem.Label = asset->GetName().CStr();
                                contentItem.DetailsDesc = asset->GetAssetType()->GetSimpleName().CStr();
                                contentItem.SetPayload = ASSET_PAYLOAD;
                                contentItem.TooltipText = asset->GetPath().CStr();
                                //contentItem.Texture = folderTexture;

                                // if (node->updated)
                                // {
                                //     contentItem.PreLabel = "*";
                                // }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    // if (node->objectType == GetTypeID<SceneObjectAsset>())
                                    // {
                                    //     Editor::GetSceneEditor().LoadScene(node->rid);
                                    // }
                                    // else if (node->objectType == GetTypeID<GraphAsset>() || node->objectType == GetTypeID<RenderGraphAsset>())
                                    // {
                                    //     GraphEditorWindow::OpenGraphWindow(node->rid);
                                    // }
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    m_selectedItem = asset;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    Editor::CreateTransaction()->CreateAction<RenameAssetAction>(asset, ImGui::ContentRenameString());

                                    //asset->Rename(ImGui::ContentRenameString(), Editor::CreateTransaction());
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
                        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && m_openDirectory)
                        {
                            selectedDiretory = static_cast<AssetDirectory*>(m_openDirectory->GetDirectory());
                        }
                    }
                }

                if (selectedDiretory)
                {
                    SetOpenDirectory(selectedDiretory);
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

    void ProjectBrowserWindow::AssetNewFolder(const MenuItemEventData& eventData)
    {
        // ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        // RID newDirectory = projectBrowserWindow->m_assetTree.NewDirectory(projectBrowserWindow->m_openFolder, "New Folder");
        //
        // ImGui::SelectContentItem(Hash<RID>::Value(newDirectory), CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
        // ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetNewScene(const MenuItemEventData& eventData)
    {
        // ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        // RID newAsset = projectBrowserWindow->m_assetTree.NewAsset(projectBrowserWindow->m_openFolder, Repository::CreateResource<SceneObjectAsset>(), "New Scene");
        //
        // ImGui::SelectContentItem(Hash<RID>::Value(newAsset), CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
        // ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetDelete(const MenuItemEventData& eventData)
    {
        // ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        // projectBrowserWindow->m_assetTree.Delete(projectBrowserWindow->m_selectedItem);
        // projectBrowserWindow->m_selectedItem = {};
    }

    bool ProjectBrowserWindow::CheckSelectedAsset(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        return projectBrowserWindow->m_selectedItem != nullptr && projectBrowserWindow->m_selectedItem->IsAlive();
    }

    void ProjectBrowserWindow::AssetRename(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetShowInExplorer(const MenuItemEventData& eventData)
    {
        // ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        // RID itemToShow = projectBrowserWindow->m_selectedItem ? projectBrowserWindow->m_selectedItem : projectBrowserWindow->m_openFolder;
        // StringView path = ResourceAssets::GetAbsolutePath(itemToShow);
        // if (!path.Empty())
        // {
        //     Platform::ShowInExplorer(path);
        // }
    }

    void ProjectBrowserWindow::AssetNewResourceGraph(const MenuItemEventData& eventData)
    {
        // ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        // RID newAsset = projectBrowserWindow->m_assetTree.NewAsset(projectBrowserWindow->m_openFolder, Repository::CreateResource<GraphAsset>(), "New Resource Graph");
        //
        // ImGui::SelectContentItem(Hash<RID>::Value(newAsset), CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
        // ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
    }

    void ProjectBrowserWindow::AssetNewRenderGraph(const MenuItemEventData& eventData)
    {
        // ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        // RID newAsset = projectBrowserWindow->m_assetTree.NewAsset(projectBrowserWindow->m_openFolder, Repository::CreateResource<RenderGraphAsset>(), "New Render Graph");
        //
        // ImGui::SelectContentItem(Hash<RID>::Value(newAsset), CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
        // ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->m_windowId);
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
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Project Browser", .action = OpenProjectBrowser});
        Event::Bind<OnShutdown, Shutdown>();

        AddMenuItem(MenuItemCreation{.itemName = "New Folder", .icon = ICON_FA_FOLDER, .priority = 0, .action = AssetNewFolder});
        AddMenuItem(MenuItemCreation{.itemName = "New Scene", .icon = ICON_FA_GLOBE, .priority = 10, .action = AssetNewScene});
        AddMenuItem(MenuItemCreation{.itemName = "Delete", .icon = ICON_FA_TRASH, .priority = 20, .itemShortcut{.presKey = Key::Delete}, .action = AssetDelete, .enable = CheckSelectedAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Rename", .icon = ICON_FA_PEN_TO_SQUARE, .priority = 30, .itemShortcut{.presKey = Key::F2}, .action = AssetRename, .enable = CheckSelectedAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Show in Explorer", .icon = ICON_FA_FOLDER, .priority = 40, .action = AssetShowInExplorer});
        AddMenuItem(MenuItemCreation{.itemName = "Create New Asset", .icon = ICON_FA_PLUS, .priority = 150});
        AddMenuItem(MenuItemCreation{.itemName = "Create New Asset/Resource Graph", .icon = ICON_FA_DIAGRAM_PROJECT, .priority = 10, .action = AssetNewResourceGraph});
        AddMenuItem(MenuItemCreation{.itemName = "Create New Asset/Behavior Graph", .icon = ICON_FA_DIAGRAM_PROJECT, .priority = 20});
        AddMenuItem(MenuItemCreation{.itemName = "Create New Asset/Render Graph", .icon = ICON_FA_DIAGRAM_PROJECT, .priority = 30, .action = AssetNewRenderGraph});

        type.Function<&ProjectBrowserWindow::SetOpenDirectory>("SetOpenDirectory");

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Bottom,
            .createOnInit = true
        });
    }

    void ProjectBrowserWindow::OpenProjectBrowser(const MenuItemEventData& eventData)
    {
        Editor::OpenWindow(GetTypeID<ProjectBrowserWindow>());
    }

    void InitProjectBrowser()
    {
        Registry::Type<ProjectBrowserWindow>();
    }
}
