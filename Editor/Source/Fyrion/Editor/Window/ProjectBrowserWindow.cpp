#include "ProjectBrowserWindow.hpp"

#include "imgui_internal.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/StaticContent.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/Scene/Scene.hpp"

#define CONTENT_TABLE_ID 500

namespace Fyrion
{
    MenuItemContext ProjectBrowserWindow::menuItemContext = {};

    void ProjectBrowserWindow::Init(u32 id, VoidPtr userData)
    {
        folderTexture = StaticContent::GetTextureFile("Content/Images/FolderIcon.png");
        fileTexture = StaticContent::GetTextureFile("Content/Images/file.png");
        brickTexture = StaticContent::GetTextureFile("Content/Images/brickwall.jpg");
    }

    void ProjectBrowserWindow::DrawPathItems() {}

    void ProjectBrowserWindow::DrawTreeNode(AssetFile* file)
    {
        if (!file->isDirectory) return;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

        bool openDir = openTreeFolders[file->absolutePath];

        if (openDir)
        {
            ImGui::SetNextItemOpen(true);
        }

        if (openDirectory && openDirectory == file)
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

        stringCache.Append(" ").Append(file->fileName);
        bool isNodeOpen = ImGui::TreeNode(file->hash, stringCache.CStr(), flags);

        if (ImGui::BeginDragDropTarget())
        {
            // if (assetPayload.asset != nullptr && !directory->IsChildOf(assetPayload.asset) && ImGui::AcceptDragDropPayload(AssetDragDropType))
            // {
            //     Editor::CreateTransaction()->CreateAction<MoveAssetAction>(assetPayload.asset, directory)->Commit();
            // }
            ImGui::EndDragDropTarget();
        }

        // if (assetHandler->GetParent() != nullptr && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
        // {
        //     assetPayload.asset = assetHandler;
        //     ImGui::SetDragDropPayload(AssetDragDropType, &assetPayload, sizeof(AssetPayload));
        //     ImGui::Text("%s", assetHandler->GetName().CStr());
        //     ImGui::EndDragDropSource();
        // }

        if (openDir == isNodeOpen && ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            SetOpenDirectory(file);
        }

        openTreeFolders[file->absolutePath] = isNodeOpen;

        if (isNodeOpen)
        {
            for (AssetFile* childNode : file->children)
            {
                DrawTreeNode(childNode);
            }
            ImGui::TreePop();
        }
    }

    void ProjectBrowserWindow::Draw(u32 id, bool& open)
    {
        String labelCache = "";

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2      pad = style.WindowPadding;

        bool readOnly = openDirectory == nullptr;

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
                Array<FileFilter> filters;
                //assetEditor.FilterExtensions(filters);

                if (Platform::OpenDialogMultiple(paths, filters, {}) == DialogResult::OK)
                {
                    if (!paths.Empty())
                    {
                        AssetEditor::ImportAssets(openDirectory, paths);
                    }
                }
            }
            ImGui::EndDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

            DrawPathItems();

            ImGui::Spring(1.0f);

            ImGui::PopStyleColor(2);

            ImGui::SetNextItemWidth(250 * style.ScaleFactor);
            ImGui::SliderFloat("###zoom", &contentBrowserZoom, 0.4f, 5.0f, "");

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
        bool newSelection = false;

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

                for (const auto& package : AssetEditor::GetPackages())
                {
                    DrawTreeNode(package);
                }

                ImGui::EndTreeNode();
                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(27, 28, 30, 255));
                auto              padding = 0;
                ImGui::StyleVar   cellPadding(ImGuiStyleVar_CellPadding, ImVec2(padding, padding));
                ImGui::StyleVar   itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(contentBrowserZoom, contentBrowserZoom));
                ImGui::StyleVar   framePadding(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::StyleVar   browserWinPadding(ImGuiStyleVar_WindowPadding, ImVec2(5.f * style.ScaleFactor, 5.f * style.ScaleFactor));

                ImGui::BeginChild(52211, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SetWindowFontScale(contentBrowserZoom);

                if (ImGui::BeginContentTable("ProjectBrowser", contentBrowserZoom))
                {
                    AssetFile* newOpenDirectory = nullptr;

                    if (openDirectory != nullptr)
                    {
                        for (int i = 0; i < 2; ++i)
                        {
                            for (AssetFile* childNode : openDirectory->children)
                            {
                                if (!childNode->active) continue;

                                //workaround to show directories first.
                                if (i == 0 && !childNode->isDirectory) continue;
                                if (i == 1 && childNode->isDirectory) continue;

                                labelCache.Clear();

                                bool renaming = renamingItem == childNode;

                                if (!renaming && childNode->IsDirty())
                                {
                                    labelCache = "*";
                                }

                                labelCache += childNode->fileName;

                                if (!renaming)
                                {
                                    labelCache += childNode->extension;
                                }

                                ImGui::ContentItemDesc desc;
                                desc.id = reinterpret_cast<usize>(childNode);
                                desc.label = labelCache.CStr();
                                desc.texture = childNode->isDirectory ? folderTexture : brickTexture;
                                desc.renameItem = renaming;
                                desc.thumbnailScale = contentBrowserZoom;
                                desc.selected = selectedItems.Has(childNode);

                                ImGui::ContentItemState state = ImGui::ContentItem(desc);

                                if (state.clicked)
                                {
                                    if (!(ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightCtrl))))
                                    {
                                        selectedItems.Clear();
                                        lastSelectedItem = nullptr;
                                    }
                                    selectedItems.Emplace(childNode);
                                    lastSelectedItem = childNode;
                                    newSelection = true;
                                }

                                if (state.doubleClicked)
                                {
                                    if (childNode->isDirectory)
                                    {
                                        newOpenDirectory = childNode;
                                        selectedItems.Clear();
                                        lastSelectedItem = nullptr;
                                    }
                                    else if (childNode->handler)
                                    {
                                        childNode->handler->OpenAsset(childNode);
                                    }
                                }

                                if (state.renameFinish)
                                {
                                    if (!state.newName.Empty())
                                    {
                                        AssetEditor::Rename(childNode, state.newName);
                                    }
                                    renamingItem = nullptr;
                                }
                            }
                        }
                    }

                    ImGui::EndContentTable();

                    if (newOpenDirectory)
                    {
                        SetOpenDirectory(newOpenDirectory);
                    }
                }

                ImGui::SetWindowFontScale(1.0);
                ImGui::EndChild();
            }
            ImGui::EndTable();

            // if (newItemSelected != selectedItem)
            // {
            //     SetSelectedAsset(newItemSelected);
            // }
        }


        bool closePopup = false;
        if (!renamingItem && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
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

        if (!popupRes && !newSelection && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
        {
            selectedItems.Clear();
            lastSelectedItem = nullptr;
        }

        ImGui::End();
    }

    void ProjectBrowserWindow::SetOpenDirectory(AssetFile* directory)
    {
        openDirectory = directory;
        if (directory && directory->parent)
        {
            openTreeFolders[directory->parent->absolutePath] = true;
        }
    }

    bool ProjectBrowserWindow::CheckSelectedAsset(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        return projectBrowserWindow->lastSelectedItem != nullptr;
    }

    void ProjectBrowserWindow::AssetRename(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        projectBrowserWindow->renamingItem = projectBrowserWindow->lastSelectedItem;
    }

    void ProjectBrowserWindow::Shutdown()
    {
        menuItemContext = MenuItemContext{};
    }

    void ProjectBrowserWindow::AssetNewFolder(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        AssetFile*            newDirectory = AssetEditor::CreateDirectory(projectBrowserWindow->openDirectory);

        projectBrowserWindow->renamingItem = newDirectory;
        projectBrowserWindow->selectedItems.Clear();
        projectBrowserWindow->selectedItems.Insert(newDirectory);
        projectBrowserWindow->lastSelectedItem = newDirectory;
    }

    void ProjectBrowserWindow::AssetNewScene(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);

        if (AssetFile* newAsset = AssetEditor::CreateAsset(projectBrowserWindow->openDirectory, GetTypeID<Scene>()))
        {
            projectBrowserWindow->renamingItem = newAsset;
            projectBrowserWindow->selectedItems.Clear();
            projectBrowserWindow->selectedItems.Insert(newAsset);
            projectBrowserWindow->lastSelectedItem = newAsset;
        }
    }

    void ProjectBrowserWindow::AssetNewMaterial(const MenuItemEventData& eventData) {}

    void ProjectBrowserWindow::AssetDelete(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);

        Array<AssetFile*> assets;
        assets.Reserve(projectBrowserWindow->selectedItems.Size());

        for (auto it: projectBrowserWindow->selectedItems)
        {
            assets.EmplaceBack(it.first);
        }

        AssetEditor::DeleteAssets(assets);
    }

    void ProjectBrowserWindow::AssetShowInExplorer(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        if (projectBrowserWindow->lastSelectedItem)
        {
            Platform::ShowInExplorer(projectBrowserWindow->lastSelectedItem->absolutePath);
        }
        else if (projectBrowserWindow->openDirectory)
        {
            Platform::ShowInExplorer(projectBrowserWindow->openDirectory->absolutePath);
        }
    }

    void ProjectBrowserWindow::AssetCopyPathToClipboard(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetNewResourceGraph(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetNewRenderGraph(const MenuItemEventData& eventData) {}

    bool ProjectBrowserWindow::CheckCanReimport(const MenuItemEventData& eventData)
    {
        return false;
    }

    void ProjectBrowserWindow::AssetReimport(const MenuItemEventData& eventData) {}

    void ProjectBrowserWindow::AssetNew(const MenuItemEventData& eventData) {}

    ProjectBrowserWindow::~ProjectBrowserWindow()
    {
        Graphics::WaitQueue();
        Graphics::DestroyTexture(folderTexture);
        Graphics::DestroyTexture(fileTexture);
        Graphics::DestroyTexture(brickTexture);
    }

    void ProjectBrowserWindow::OpenProjectBrowser(const MenuItemEventData& eventData)
    {
        Editor::OpenWindow(GetTypeID<ProjectBrowserWindow>());
    }

    void ProjectBrowserWindow::AddMenuItem(const MenuItemCreation& menuItem)
    {
        menuItemContext.AddMenuItem(menuItem);
    }

    void ProjectBrowserWindow::RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type)
    {
        Event::Bind<OnShutdown, Shutdown>();

        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Project Browser", .action = OpenProjectBrowser});

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


        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Bottom,
            .createOnInit = true
        });
    }
}
