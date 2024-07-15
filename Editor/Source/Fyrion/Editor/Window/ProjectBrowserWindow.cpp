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
#include "Fyrion/Editor/Action/SceneEditorAction.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"

#define CONTENT_TABLE_ID 500
#define ASSET_PAYLOAD "ASSET-PAYLOAD"

namespace Fyrion
{

    MenuItemContext ProjectBrowserWindow::menuItemContext = {};

    void ProjectBrowserWindow::Init(u32 id, VoidPtr userData)
    {
        windowId = id;
        if (!Editor::GetOpenDirectories().Empty())
        {
            SetOpenDirectory(Editor::GetOpenDirectories().Back());
        }

        if (TextureAsset* texture = AssetDatabase::FindByPath<TextureAsset>("Fyrion://Textures/FolderIcon.png"))
        {
            folderTexture = texture->GetTexture();
        }

        if (TextureAsset* texture = AssetDatabase::FindByPath<TextureAsset>("Fyrion://Textures/FileIcon.png"))
        {
            fileTexture = texture->GetTexture();
        }
    }

    void ProjectBrowserWindow::SetOpenDirectory(AssetDirectory* p_directory)
    {
        openDirectory = p_directory;
        selectedItem = {};

        if (p_directory->GetDirectory() != nullptr)
        {
            openTreeFolders[reinterpret_cast<usize>(p_directory->GetDirectory())] = true;
        }
    }

    void ProjectBrowserWindow::DrawTreeNode(Asset* asset)
    {
        if (asset == nullptr || !asset->IsActive()) return;

        AssetDirectory* directory = asset->GetAssetType()->Cast<AssetDirectory>(asset);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

        bool openDir = openTreeFolders[reinterpret_cast<usize>(asset)];

        if (openDir)
        {
            ImGui::SetNextItemOpen(true);
        }

        if (openDirectory == asset)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        stringCache.Clear();
        if (openDir)
        {
            stringCache = ICON_FA_FOLDER_OPEN;
        }
        else
        {
            stringCache = ICON_FA_FOLDER;
        }

        stringCache.Append(" ").Append(asset->GetName());

        bool isNodeOpen = ImGui::TreeNode(HashValue(reinterpret_cast<usize>(asset)), stringCache.CStr(), flags);

        if (ImGui::BeginDragDropTarget())
        {
            if (movingItem != nullptr && !directory->IsChildOf(movingItem) && ImGui::AcceptDragDropPayload(ASSET_PAYLOAD))
            {
                Editor::CreateTransaction()->CreateAction<MoveAssetAction>(movingItem, directory)->Commit();
            }
            ImGui::EndDragDropTarget();
        }

        if (asset->GetDirectory() != nullptr && ImGui::BeginDragDropSource())
        {
            movingItem = asset;
            ImGui::SetDragDropPayload(ASSET_PAYLOAD, nullptr, 0);
            ImGui::Text("%s", asset->GetName().CStr());
            ImGui::EndDragDropSource();
        }

        if (openDir == isNodeOpen && ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            SetOpenDirectory(directory);
        }

        openTreeFolders[reinterpret_cast<usize>(directory)] = isNodeOpen;

        if (isNodeOpen)
        {
            for (Asset* child : directory->GetChildren())
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
        if (!openDirectory) return;
        directoryCache.Clear();

        {
            AssetDirectory* item = openDirectory;
            while (item != nullptr)
            {
                directoryCache.EmplaceBack(item);
                item = item->GetDirectory();
            }
        }

        AssetDirectory* nextDirectory = nullptr;

        for (usize i = directoryCache.Size(); i > 0; --i)
        {
            AssetDirectory* drawItem = directoryCache[i - 1];

            if (ImGui::Button(drawItem->GetName().CStr()))
            {
                nextDirectory = drawItem;
            }

            if (i > 1)
            {
                bool openPopup = false;
                ImGui::PushID(static_cast<int>(reinterpret_cast<usize>(drawItem)));
                if (ImGui::Button(ICON_FA_ARROW_RIGHT))
                {
                    openPopup = true;
                }
                ImGui::PopID();

                if (openPopup)
                {
                    popupFolder = drawItem;
                    ImGui::OpenPopup("select-folder-browser-popup");
                }
            }
        }

        if (nextDirectory)
        {
            SetOpenDirectory(nextDirectory);
        }

        auto popupRes = ImGui::BeginPopupMenu("select-folder-browser-popup");
        if (popupRes && popupFolder)
        {
            for (Asset* node : popupFolder->GetChildren())
            {
                if (AssetDirectory* assetDirectory = dynamic_cast<AssetDirectory*>(node))
                {
                    if (ImGui::MenuItem(assetDirectory->GetName().CStr()))
                    {
                        SetOpenDirectory(assetDirectory);
                    }
                }
            }
        }
        ImGui::EndPopupMenu(popupRes);
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
                            AssetDatabase::ImportAsset(openDirectory, path);
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
            ImGui::SearchInputText(id + 20, searchString);


            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

            if (ImGui::Button(ICON_FA_GEAR " Settings")) {}

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

                ImGui::SetWindowFontScale(contentBrowserZoom);

                AssetDirectory* selectedDiretory = nullptr;

                if (ImGui::BeginContentTable(id + CONTENT_TABLE_ID, contentBrowserZoom * 112 * ImGui::GetStyle().ScaleFactor))
                {
                    if (openDirectory)
                    {
                        for (Asset* asset : openDirectory->GetChildren())
                        {
                            if (!asset->IsActive()) continue;

                            if (asset->GetAssetTypeId() == GetTypeID<AssetDirectory>())
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId = reinterpret_cast<usize>(asset);
                                contentItem.ShowDetails = false;
                                contentItem.Label = asset->GetName().CStr();
                                contentItem.SetPayload = ASSET_PAYLOAD;
                                contentItem.AcceptPayload = ASSET_PAYLOAD;
                                contentItem.TooltipText = asset->GetPath().CStr();
                                contentItem.Texture = folderTexture;

                                if (asset->IsModified())
                                {
                                    contentItem.PreLabel = "*";
                                }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    ImGui::ContentItemSelected(U32_MAX);
                                    selectedDiretory = asset->GetAssetType()->Cast<AssetDirectory>(asset);
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    selectedItem = asset;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    if (ImGui::ContentRenameString() != asset->GetName())
                                    {
                                        Editor::CreateTransaction()->CreateAction<RenameAssetAction>(asset, ImGui::ContentRenameString())->Commit();
                                    }
                                }

                                if (!asset->IsChildOf(movingItem) && ImGui::ContentItemAcceptPayload(contentItem.ItemId))
                                {
                                    Editor::CreateTransaction()->CreateAction<MoveAssetAction>(movingItem, dynamic_cast<AssetDirectory*>(asset))->Commit();
                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    movingItem = selectedItem;
                                }
                            }
                        }

                        for (Asset* asset : openDirectory->GetChildren())
                        {
                            if (!asset->IsActive()) continue;

                            if (asset->GetAssetTypeId() != GetTypeID<AssetDirectory>())
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId = reinterpret_cast<usize>(asset);
                                contentItem.ShowDetails = true;
                                contentItem.Label = asset->GetName().CStr();
                                contentItem.DetailsDesc = asset->GetDisplayName().CStr();
                                contentItem.SetPayload = ASSET_PAYLOAD;
                                contentItem.TooltipText = asset->GetPath().CStr();
                                contentItem.Texture = fileTexture;

                                if (asset->IsModified())
                                {
                                    contentItem.PreLabel = "*";
                                }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    if (SceneObjectAsset* sceneObjectAsset = dynamic_cast<SceneObjectAsset*>(asset))
                                    {
                                        Editor::CreateTransaction()->CreateAction<OpenSceneAction>(Editor::GetSceneEditor(), sceneObjectAsset)->Commit();
                                    }
                                    // else if (node->objectType == GetTypeID<GraphAsset>() || node->objectType == GetTypeID<RenderGraphAsset>())
                                    // {
                                    //     GraphEditorWindow::OpenGraphWindow(node->rid);
                                    // }
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    selectedItem = asset;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    if (ImGui::ContentRenameString() != asset->GetName())
                                    {
                                        Editor::CreateTransaction()->CreateAction<RenameAssetAction>(asset, ImGui::ContentRenameString())->Commit();
                                    }

                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    movingItem = selectedItem;
                                }
                            }
                        }
                    }
                    ImGui::EndContentTable();

                    if (!ImGui::RenamingSelected(CONTENT_TABLE_ID + id) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
                    {
                        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && openDirectory)
                        {
                            selectedDiretory = openDirectory->GetDirectory();
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
            if (menuItemContext.ExecuteHotKeys(this))
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
            menuItemContext.Draw(this);
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
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<AssetDirectory>());
    }

    void ProjectBrowserWindow::AssetNewScene(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<SceneObjectAsset>());
    }

    void ProjectBrowserWindow::AssetDelete(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        Editor::CreateTransaction()->CreateAction<AssetDeleteAction>(projectBrowserWindow->selectedItem)->Commit();
        projectBrowserWindow->selectedItem = {};
    }

    bool ProjectBrowserWindow::CheckSelectedAsset(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        return projectBrowserWindow->selectedItem != nullptr && projectBrowserWindow->selectedItem->IsActive();
    }

    void ProjectBrowserWindow::AssetRename(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->windowId);
    }

    void ProjectBrowserWindow::AssetShowInExplorer(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        Asset*                itemToShow = projectBrowserWindow->selectedItem ? projectBrowserWindow->selectedItem : projectBrowserWindow->openDirectory;
        if (itemToShow != nullptr)
        {
            Platform::ShowInExplorer(itemToShow->GetAbsolutePath());
        }
    }

    void ProjectBrowserWindow::AssetCopyPathToClipboard(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        Asset* item = projectBrowserWindow->selectedItem ? projectBrowserWindow->selectedItem : projectBrowserWindow->openDirectory;
        if (item != nullptr)
        {
            Platform::SetClipboardString(Engine::GetActiveWindow(), item->GetPath());
        }
    }

    void ProjectBrowserWindow::AssetNewResourceGraph(const MenuItemEventData& eventData)
    {
    }

    void ProjectBrowserWindow::AssetNewRenderGraph(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<RenderGraphAsset>());
    }

    void ProjectBrowserWindow::AssetNew(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(reinterpret_cast<TypeID>(eventData.itemData));
    }

    void ProjectBrowserWindow::NewAsset(TypeID typeId)
    {
        AssetCreateAction*    assetCreationAction = Editor::CreateTransaction()->CreateAction<AssetCreateAction>(openDirectory, typeId);
        assetCreationAction->Commit();
        ImGui::SelectContentItem(reinterpret_cast<usize>(assetCreationAction->GetNewAsset()), CONTENT_TABLE_ID + windowId);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + windowId);
    }

    void ProjectBrowserWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        menuItemContext.AddMenuItem(menuItem);
    }

    void ProjectBrowserWindow::Shutdown()
    {
        menuItemContext = MenuItemContext{};
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
        AddMenuItem(MenuItemCreation{.itemName = "Copy Path", .priority = 1000, .action = AssetCopyPathToClipboard});
        AddMenuItem(MenuItemCreation{.itemName = "Create New Asset", .icon = ICON_FA_PLUS, .priority = 150});
        // AddMenuItem(MenuItemCreation{.itemName = "Create New Asset/Shader", .icon = ICON_FA_BRUSH, .priority = 10, .action = AssetNew, .menuData = (VoidPtr)GetTypeID<ShaderAsset>()});
        // AddMenuItem(MenuItemCreation{.itemName = "Create New Asset/Resource Graph", .icon = ICON_FA_DIAGRAM_PROJECT, .priority = 10, .action = AssetNewResourceGraph});
        // AddMenuItem(MenuItemCreation{.itemName = "Create New Asset/Behavior Graph", .icon = ICON_FA_DIAGRAM_PROJECT, .priority = 20});
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
