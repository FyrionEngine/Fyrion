#include "ProjectBrowserWindow.hpp"

#include "GraphEditorWindow.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetManager.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Editor/Action/AssetEditorActions.hpp"
#include "Fyrion/Editor/Action/SceneEditorAction.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/MaterialAsset.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"

#define CONTENT_TABLE_ID 500

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

        if (TextureAsset* texture = AssetManager::LoadByPath<TextureAsset>("Fyrion://Textures/FolderIcon.png"))
        {
            folderTexture = texture->GetTexture();
        }

        if (TextureAsset* texture = AssetManager::LoadByPath<TextureAsset>("Fyrion://Textures/FileIcon.png"))
        {
            fileTexture = texture->GetTexture();
        }
    }

    void ProjectBrowserWindow::SetOpenDirectory(DirectoryInfo* directory)
    {
        openDirectory = directory;
        selectedItem = {};

        if (directory->GetParent() != nullptr)
        {
            openTreeFolders[reinterpret_cast<usize>(directory->GetParent())] = true;
        }

        lastOpenedDirectory = openDirectory;
    }

    void ProjectBrowserWindow::SetSelectedAsset(AssetInfo* selectedItem)
    {
        this->selectedItem = selectedItem;
        onAssetSelectionHandler.Invoke(this->selectedItem);

    }

    void ProjectBrowserWindow::DrawTreeNode(AssetInfo* assetInfo)
    {
        if (assetInfo == nullptr) return;

        DirectoryInfo* directory =  dynamic_cast<DirectoryInfo*>(assetInfo);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

        bool openDir = openTreeFolders[reinterpret_cast<usize>(assetInfo)];

        if (openDir)
        {
            ImGui::SetNextItemOpen(true);
        }

        if (openDirectory == assetInfo)
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

        stringCache.Append(" ").Append(assetInfo->GetName());

        bool isNodeOpen = ImGui::TreeNode(HashValue(reinterpret_cast<usize>(assetInfo)), stringCache.CStr(), flags);

        if (ImGui::BeginDragDropTarget())
        {
            if (assetPayload.asset != nullptr && !directory->IsChildOf(assetPayload.asset) && ImGui::AcceptDragDropPayload(AssetDragDropType))
            {
                Editor::CreateTransaction()->CreateAction<MoveAssetAction>(assetPayload.asset, directory)->Commit();
            }
            ImGui::EndDragDropTarget();
        }

        if (assetInfo->GetParent() != nullptr && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
        {
            assetPayload.asset = assetInfo;
            ImGui::SetDragDropPayload(AssetDragDropType, &assetPayload, sizeof(AssetPayload));
            ImGui::Text("%s", assetInfo->GetName().CStr());
            ImGui::EndDragDropSource();
        }

        if (openDir == isNodeOpen && ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            SetOpenDirectory(directory);
        }

        openTreeFolders[reinterpret_cast<usize>(directory)] = isNodeOpen;

        if (isNodeOpen)
        {
            for (AssetInfo* child : directory->GetChildren())
            {
                if (DirectoryInfo* childDirectory = dynamic_cast<DirectoryInfo*>(child))
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
            DirectoryInfo* item = openDirectory;
            while (item != nullptr)
            {
                directoryCache.EmplaceBack(item);
                item = dynamic_cast<DirectoryInfo*>(item->GetParent());
            }
        }

        DirectoryInfo* nextDirectory = nullptr;

        for (usize i = directoryCache.Size(); i > 0; --i)
        {
            DirectoryInfo* drawItem = directoryCache[i - 1];

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
            for (AssetInfo* node : popupFolder->GetChildren())
            {
                if (DirectoryInfo* assetDirectory = dynamic_cast<DirectoryInfo*>(node))
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

        bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        //top child
        {
            ImVec2          a = ImVec2(pad.x / 1.5f, pad.y / 1.5f);
            ImGui::StyleVar childPadding(ImGuiStyleVar_WindowPadding, a);
            f32             width = ImGui::GetContentRegionAvail().x - a.x;
            ImGui::BeginChild(id + 5, ImVec2(width, 30 * style.ScaleFactor), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);

            ImGui::BeginHorizontal((i32)id + 10, ImVec2(width - a.x - pad.x, 0));

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
                            AssetManager::ImportAsset(openDirectory, path);
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

        bool browseFolder = true;

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

                for (DirectoryInfo* directory : Editor::GetOpenDirectories())
                {
                    DrawTreeNode(directory);
                }

                ImGui::EndTreeNode();
                ImGui::EndChild();
            }

            AssetInfo* newItemSelected = nullptr;

            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(27, 28, 30, 255));
                auto              padding = 5.f * style.ScaleFactor;
                ImGui::StyleVar   cellPadding(ImGuiStyleVar_CellPadding, ImVec2(padding, padding));
                ImGui::StyleVar   browserWinPadding(ImGuiStyleVar_WindowPadding, ImVec2(padding * 2, padding));

                ImGui::BeginChild(52211, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SetWindowFontScale(contentBrowserZoom);

                DirectoryInfo* selectedDiretory = nullptr;

                if (ImGui::BeginContentTable(id + CONTENT_TABLE_ID, contentBrowserZoom * 112 * ImGui::GetStyle().ScaleFactor))
                {
                    if (openDirectory)
                    {
                        for (AssetInfo* asset : openDirectory->GetChildren())
                        {
                            if (DirectoryInfo* directoryInfo = dynamic_cast<DirectoryInfo*>(asset))
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId = reinterpret_cast<usize>(asset);
                                contentItem.ShowDetails = false;
                                contentItem.Label = asset->GetName().CStr();
                                contentItem.DragDropType = AssetDragDropType;
                                contentItem.AcceptPayload = AssetDragDropType;
                                contentItem.TooltipText = asset->GetPath().CStr();
                                contentItem.Texture = folderTexture;

                                if (asset->IsModified())
                                {
                                    contentItem.PreLabel = "*";
                                }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    ImGui::ContentItemSelected(U32_MAX);
                                    selectedDiretory = directoryInfo;
                                }

                                if (ImGui::ContentItemSelected(contentItem.ItemId))
                                {
                                    newItemSelected = asset;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    if (ImGui::ContentRenameString() != asset->GetName())
                                    {
                                        Editor::CreateTransaction()->CreateAction<RenameAssetAction>(asset, ImGui::ContentRenameString())->Commit();
                                    }
                                }

                                if (!asset->IsChildOf(assetPayload.asset) && ImGui::ContentItemAcceptPayload(contentItem.ItemId))
                                {
                                    Editor::CreateTransaction()->CreateAction<MoveAssetAction>(assetPayload.asset, dynamic_cast<DirectoryInfo*>(asset))->Commit();
                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    assetPayload.asset = selectedItem;
                                }
                            }
                        }

                        for (AssetInfo* assetInfo : openDirectory->GetChildren())
                        {
                            if (DirectoryInfo* directoryInfo = dynamic_cast<DirectoryInfo*>(assetInfo); directoryInfo == nullptr)
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.ItemId = reinterpret_cast<usize>(assetInfo);
                                contentItem.ShowDetails = true;
                                contentItem.Label = assetInfo->GetName().CStr();
                                contentItem.DetailsDesc = assetInfo->GetDisplayName().CStr();
                                contentItem.DragDropType = AssetDragDropType;
                                contentItem.DragDropPayload = &assetPayload;
                                contentItem.DragDropPayloadSize = sizeof(AssetPayload);
                                contentItem.TooltipText = assetInfo->GetPath().CStr();
                                contentItem.Texture = fileTexture;

                                if (assetInfo->IsModified())
                                {
                                    contentItem.PreLabel = "*";
                                }

                                if (ImGui::DrawContentItem(contentItem))
                                {
                                    if (SceneObjectAsset* sceneObjectAsset = dynamic_cast<SceneObjectAsset*>(assetInfo))
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
                                    newItemSelected = assetInfo;
                                }

                                if (ImGui::ContentItemRenamed(contentItem.ItemId))
                                {
                                    if (ImGui::ContentRenameString() != assetInfo->GetName())
                                    {
                                        Editor::CreateTransaction()->CreateAction<RenameAssetAction>(assetInfo, ImGui::ContentRenameString())->Commit();
                                    }
                                }

                                if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                                {
                                    assetPayload.asset = selectedItem;
                                }
                            }
                        }
                    }
                    ImGui::EndContentTable();

                    if (!ImGui::RenamingSelected(CONTENT_TABLE_ID + id) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
                    {
                        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && openDirectory)
                        {
                            selectedDiretory = dynamic_cast<DirectoryInfo*>(openDirectory->GetParent());
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

            if (newItemSelected != selectedItem)
            {
                SetSelectedAsset(newItemSelected);
            }
        }


        // if (selectedItem != focusItem)
        // {
        //     if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        //     {
        //         onAssetSelectionHandler.Invoke(selectedItem);
        //         focusItem = selectedItem;
        //     }
        //
        //     if (!hovered)
        //     {
        //         selectedItem = focusItem;
        //         ImGui::SelectContentItem(reinterpret_cast<usize>(selectedItem), CONTENT_TABLE_ID + windowId);
        //     }
        // }

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
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<DirectoryInfo>());
    }

    void ProjectBrowserWindow::AssetNewScene(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<SceneObjectAsset>());
    }

    void ProjectBrowserWindow::AssetNewMaterial(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<MaterialAsset>());
    }

    void ProjectBrowserWindow::AssetDelete(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
      //  projectBrowserWindow->selectedItem->Destroy();
        projectBrowserWindow->SetSelectedAsset(nullptr);
    }

    bool ProjectBrowserWindow::CheckSelectedAsset(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        return projectBrowserWindow->selectedItem != nullptr;
    }

    void ProjectBrowserWindow::AssetRename(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        ImGui::RenameContentSelected(CONTENT_TABLE_ID + projectBrowserWindow->windowId);
    }

    void ProjectBrowserWindow::AssetShowInExplorer(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        AssetInfo*            itemToShow = projectBrowserWindow->selectedItem ? projectBrowserWindow->selectedItem : projectBrowserWindow->openDirectory;

        if (itemToShow != nullptr)
        {
            Platform::ShowInExplorer(itemToShow->GetAbsolutePath());
        }
    }

    void ProjectBrowserWindow::AssetCopyPathToClipboard(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);

        AssetInfo* item = projectBrowserWindow->selectedItem ? projectBrowserWindow->selectedItem : projectBrowserWindow->openDirectory;

        if (item != nullptr)
        {
            Platform::SetClipboardString(Engine::GetActiveWindow(), item->GetPath());
        }
    }

    void ProjectBrowserWindow::AssetNewResourceGraph(const MenuItemEventData& eventData) {}

    void ProjectBrowserWindow::AssetNewRenderGraph(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(GetTypeID<RenderGraphAsset>());
    }

    bool ProjectBrowserWindow::CheckCanReimport(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        return CheckSelectedAsset(eventData) && AssetManager::CanReimportAsset(projectBrowserWindow->selectedItem);
    }

    void ProjectBrowserWindow::AssetReimport(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        AssetManager::ReimportAsset(projectBrowserWindow->selectedItem);
    }

    void ProjectBrowserWindow::AssetNew(const MenuItemEventData& eventData)
    {
        static_cast<ProjectBrowserWindow*>(eventData.drawData)->NewAsset(reinterpret_cast<TypeID>(eventData.itemData));
    }

    void ProjectBrowserWindow::NewAsset(TypeID typeId)
    {
        // Asset* asset = AssetManager::Create(Registry::FindTypeById(typeId), {
        //     .parent = openDirectory,
        //     .generateName = true,
        // });
        //
        // ImGui::SelectContentItem(reinterpret_cast<usize>(asset), CONTENT_TABLE_ID + windowId);
        // ImGui::RenameContentSelected(CONTENT_TABLE_ID + windowId);
    }

    void ProjectBrowserWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        menuItemContext.AddMenuItem(menuItem);
    }

    void ProjectBrowserWindow::Shutdown()
    {
        menuItemContext = MenuItemContext{};
    }

    void ProjectBrowserWindow::DropFileCallback(Window window, const StringView& path)
    {
        if (lastOpenedDirectory)
        {
            AssetManager::ImportAsset(lastOpenedDirectory, path);
        }
    }

    void ProjectBrowserWindow::RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Project Browser", .action = OpenProjectBrowser});
        Event::Bind<OnShutdown, Shutdown>();
        Event::Bind<OnDropFileCallback, DropFileCallback>();

        AddMenuItem(MenuItemCreation{.itemName = "New Folder", .icon = ICON_FA_FOLDER, .priority = 0, .action = AssetNewFolder});
        AddMenuItem(MenuItemCreation{.itemName = "New Scene", .icon = ICON_FA_GLOBE, .priority = 10, .action = AssetNewScene});
        AddMenuItem(MenuItemCreation{.itemName = "New Material", .icon = ICON_FA_PAINTBRUSH, .priority = 15, .action = AssetNewMaterial});
        AddMenuItem(MenuItemCreation{.itemName = "Delete", .icon = ICON_FA_TRASH, .priority = 20, .itemShortcut{.presKey = Key::Delete}, .action = AssetDelete, .enable = CheckSelectedAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Rename", .icon = ICON_FA_PEN_TO_SQUARE, .priority = 30, .itemShortcut{.presKey = Key::F2}, .action = AssetRename, .enable = CheckSelectedAsset});
        AddMenuItem(MenuItemCreation{.itemName = "Show in Explorer", .icon = ICON_FA_FOLDER, .priority = 40, .action = AssetShowInExplorer});
        AddMenuItem(MenuItemCreation{.itemName = "Reimport", .icon = ICON_FA_REPEAT, .priority = 50, .action = AssetReimport, .enable = CheckCanReimport});
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
