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

#define CONTENT_TABLE_ID 500

namespace Fyrion
{
    MenuItemContext ProjectBrowserWindow::menuItemContext = {};


    void ProjectBrowserWindow::Init(u32 id, VoidPtr userData)
    {
        for (const String& package : Editor::GetOpenPackages())
        {
            assetEditor.AddDirectory(package);
        }

        folderTexture = StaticContent::GetTextureFile("Content/Images/FolderIcon.png");
        fileTexture = StaticContent::GetTextureFile("Content/Images/FileIcon.png");
        brickTexture = StaticContent::GetTextureFile("Content/Images/brickwall.jpg");
    }

    void ProjectBrowserWindow::DrawPathItems()
    {

    }

    void ProjectBrowserWindow::DrawTreeNode(const AssetCache& node)
    {
        if (!node.isDirectory) return;


        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

        bool openDir = openTreeFolders[node.absolutePath];

        if (openDir)
        {
            ImGui::SetNextItemOpen(true);
        }

        if (!openDirectory.Empty() && openDirectory == node.absolutePath)
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

        stringCache.Append(" ").Append(node.fileName);
        bool isNodeOpen = ImGui::TreeNode(node.hash, stringCache.CStr(), flags);

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
            SetOpenDirectory(node.absolutePath);
        }

        openTreeFolders[node.absolutePath] = isNodeOpen;

        if (isNodeOpen)
        {
            for (const auto& childNode : node.children)
            {
                DrawTreeNode(*childNode);
            }
            ImGui::TreePop();
        }
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
                          //  AssetManager::ImportAsset(openDirectory, path);
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

                for (const auto& package : assetEditor.GetDirectories())
                {
                    DrawTreeNode(*package);
                }

                ImGui::EndTreeNode();
                ImGui::EndChild();
            }

            //AssetHandler* newItemSelected = nullptr;


            ImGui::TableNextColumn();
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(27, 28, 30, 255));
                //auto              padding = 5.f * style.ScaleFactor;
                auto padding = 0;
                ImGui::StyleVar   cellPadding(ImGuiStyleVar_CellPadding, ImVec2(padding, padding));
                ImGui::StyleVar   browserWinPadding(ImGuiStyleVar_WindowPadding, ImVec2(5.f * style.ScaleFactor, 5.f * style.ScaleFactor));

                ImGui::BeginChild(52211, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

                ImGui::SetWindowFontScale(contentBrowserZoom);


                u32 thumbnailSize = (contentBrowserZoom * 112.f) * ImGui::GetStyle().ScaleFactor;
                if (ImGui::BeginContentTable("ProjectBrowser", thumbnailSize))
                {
                    String newOpenDirectory = "";

                    if (const AssetCache* openDirectoryNode = assetEditor.FindNode(openDirectory))
                    {
                        for (const auto& childNode : openDirectoryNode->children)
                        {
                            ImGui::ContentItemDesc desc;
                            desc.id = reinterpret_cast<usize>(childNode.Get());
                            desc.label = childNode->fileName.CStr();
                            desc.texture = childNode->isDirectory ? folderTexture : fileTexture;
                            desc.renameItem = renamingItem == childNode->absolutePath;
                            desc.thumbnailSize = thumbnailSize;
                            desc.selected = selectedItems.Has(childNode->absolutePath);

                            ImGui::ContentItemState state = ImGui::ContentItem(desc);

                            if (state.clicked)
                            {
                                if (!(ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightCtrl))))
                                {
                                    selectedItems.Clear();
                                    lastSelectedItem = "";
                                }
                                selectedItems.Emplace(childNode->absolutePath);
                                lastSelectedItem = childNode->absolutePath;
                                newSelection = true;
                            }

                            if (state.doubleClicked)
                            {
                                if (childNode->isDirectory)
                                {
                                    openDirectory = childNode->absolutePath;
                                    selectedItems.Clear();
                                    lastSelectedItem = "";
                                }
                            }

                            if (state.renameFinish)
                            {
                                if (!state.newName.Empty())
                                {
                                    childNode->fileName = state.newName;
                                }
                                renamingItem = "";
                            }
                        }
                    }

                    ImGui::EndContentTable();

                    if (!newOpenDirectory.Empty())
                    {
                        openDirectory = newOpenDirectory;
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

        if (!newSelection && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
        {
            selectedItems.Clear();
            lastSelectedItem = "";
        }

        bool closePopup = false;
        if (renamingItem.Empty() && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
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

    void ProjectBrowserWindow::SetOpenDirectory(StringView directory)
    {
        openDirectory = directory;
    }

    bool ProjectBrowserWindow::CheckSelectedAsset(const MenuItemEventData& eventData)
    {
        ProjectBrowserWindow* projectBrowserWindow = static_cast<ProjectBrowserWindow*>(eventData.drawData);
        return !projectBrowserWindow->lastSelectedItem.Empty();
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
        projectBrowserWindow->renamingItem = projectBrowserWindow->assetEditor.CreateDirectory(projectBrowserWindow->openDirectory);
    }

    void ProjectBrowserWindow::AssetNewScene(const MenuItemEventData& eventData)
    {

    }

    void ProjectBrowserWindow::AssetNewMaterial(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetDelete(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetShowInExplorer(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetCopyPathToClipboard(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetNewResourceGraph(const MenuItemEventData& eventData) {}
    void ProjectBrowserWindow::AssetNewRenderGraph(const MenuItemEventData& eventData) {}

    bool ProjectBrowserWindow::CheckCanReimport(const MenuItemEventData& eventData)
    {
        return false;
    }

    void ProjectBrowserWindow::AssetReimport(const MenuItemEventData& eventData) {}

    void ProjectBrowserWindow::AssetNew(const MenuItemEventData& eventData)
    {

    }

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
