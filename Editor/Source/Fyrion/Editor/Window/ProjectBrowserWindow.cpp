#include "ProjectBrowserWindow.hpp"

#include "imgui_internal.h"
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
            fileTreeCache.AddDirectory(package);
        }

        folderTexture = StaticContent::GetTextureFile("Content/Images/FolderIcon.png");
        fileTexture = StaticContent::GetTextureFile("Content/Images/FileIcon.png");
        brickTexture = StaticContent::GetTextureFile("Content/Images/brickwall.jpg");
    }

    void ProjectBrowserWindow::DrawPathItems()
    {

    }

    void ProjectBrowserWindow::DrawTreeNode(const FileTreeCacheNode& node)
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

    void ProjectBrowserWindow::SetOpenDirectory(StringView directory)
    {
        openDirectory = directory;
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

                for (const auto& package : fileTreeCache.GetDirectories())
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

                u32 thumbnailSize = 112 * ImGui::GetStyle().ScaleFactor;

                static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedSame;
                u32 columns = Math::Max(static_cast<u32>((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x) / thumbnailSize), 1u);

                if (ImGui::BeginTable("ContentTable", columns, tableFlags))
                {
                    for (int i = 0; i < columns; ++i)
                    {
                        char buffer[20]{};
                        StringConverter<i32>::ToString(buffer, 0, i);
                        ImGui::TableSetupColumn(buffer, ImGuiTableColumnFlags_WidthFixed, thumbnailSize);
                    }


                    String newOpenDirectory = "";


                    if (const FileTreeCacheNode* openDirectoryNode = fileTreeCache.FindNode(openDirectory))
                    {
                        for (const auto& childNode : openDirectoryNode->children)
                        {
                            ImGui::TableNextColumn();

                            auto cursorPos = ImGui::GetCursorScreenPos();

                            //texture
                            f32 imagePadding = thumbnailSize * 0.08f;
                            ImGui::DrawTexture(childNode->isDirectory ? folderTexture : brickTexture, {
                                          i32(cursorPos.x + imagePadding * 2),
                                          i32(cursorPos.y + imagePadding),
                                          (u32)(cursorPos.x + thumbnailSize - imagePadding * 2),
                                          (u32)(cursorPos.y + thumbnailSize - imagePadding * 3)
                                      });

                            //rect size
                            ImGui::ItemSize(ImVec2{(f32)thumbnailSize, (f32)thumbnailSize - imagePadding * 2});

                            ImGui::BeginHorizontal(&childNode, ImVec2(thumbnailSize, 0.0f));
                            ImGui::Spring();

                            //text
                            ImGui::Text("%s", childNode->fileName.CStr());

                            ImGui::Spring();
                            ImGui::EndHorizontal();

                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * ImGui::GetStyle().ScaleFactor);

                            auto posIni = ImVec2(cursorPos.x, cursorPos.y);
                            auto posEnd = ImVec2(cursorPos.x + thumbnailSize, ImGui::GetCursorScreenPos().y - 1);

                            bool hovered = ImGui::IsMouseHoveringRect(posIni, posEnd, true);

                            if (hovered)
                            {
                                drawList->AddRectFilled(posIni,
                                                        posEnd,
                                                        IM_COL32(40, 41, 43, 255),
                                                        0.0f);
                            }

                            i32  mouseCount = ImGui::GetMouseClickedCount(ImGuiMouseButton_Left);
                            bool isDoubleClicked = mouseCount >= 2 && (mouseCount % 2) == 0 && hovered;
                            bool isClicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && hovered;

                            if (isClicked)
                            {
                                selectedItem = childNode->absolutePath;
                            }

                            if (isDoubleClicked)
                            {
                                if (childNode->isDirectory)
                                {
                                    openDirectory = childNode->absolutePath;
                                    selectedItem = "";
                                }
                            }




                            if (childNode->absolutePath == selectedItem)
                            {
                                //selected
                                drawList->AddRect(ImVec2(cursorPos.x, cursorPos.y),
                                                  ImVec2(cursorPos.x + thumbnailSize, ImGui::GetCursorScreenPos().y - 1),
                                                 ImGui::ColorConvertFloat4ToU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f)),
                                                  0.0f, 0, 2);
                            }




                        }
                    }

                    ImGui::EndTable();

                    if (!newOpenDirectory.Empty())
                    {
                        openDirectory = newOpenDirectory;
                    }
                }





                // //DirectoryAssetHandler* selectedDiretory = nullptr;
                //
                // if (ImGui::BeginContentTable(id + CONTENT_TABLE_ID, contentBrowserZoom * 112 * ImGui::GetStyle().ScaleFactor))
                // {
                //     if (const FileTreeCacheNode* openDirectoryNode = fileTreeCache.FindNode(openDirectory))
                //     {
                //         for (const auto& childNode : openDirectoryNode->children)
                //         {
                //             if (!childNode->isDirectory) continue;
                //
                //             ImGui::ContentItemDesc contentItem{};
                //             contentItem.ItemId = childNode->hash;
                //             contentItem.ShowDetails = false;
                //             contentItem.Label = childNode->fileName.CStr();
                //             contentItem.DragDropType = AssetDragDropType;
                //             contentItem.AcceptPayload = AssetDragDropType;
                //             contentItem.Texture = folderTexture;
                //
                //             if (ImGui::DrawContentItem(contentItem))
                //             {
                //                 ImGui::ContentItemSelected(U32_MAX);
                //             }
                //
                //             if (ImGui::ContentItemSelected(contentItem.ItemId))
                //             {
                //                 //newItemSelected = asset;
                //             }
                //
                //             if (ImGui::ContentItemRenamed(contentItem.ItemId))
                //             {
                //                 // if (ImGui::ContentRenameString() != asset->GetName())
                //                 // {
                //                 //     Editor::CreateTransaction()->CreateAction<RenameAssetAction>(asset, ImGui::ContentRenameString())->Commit();
                //                 // }
                //             }
                //
                //             // if (!asset->IsChildOf(assetPayload.asset) && ImGui::ContentItemAcceptPayload(contentItem.ItemId))
                //             // {
                //             //     Editor::CreateTransaction()->CreateAction<MoveAssetAction>(assetPayload.asset, dynamic_cast<DirectoryAssetHandler*>(asset))->Commit();
                //             // }
                //
                //             // if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                //             // {
                //             //     assetPayload.asset = selectedItem;
                //             //}
                //         }
                //
                //         for (const auto& childNode : openDirectoryNode->children)
                //         {
                //             if (childNode->isDirectory) continue;
                //
                //             ImGui::ContentItemDesc contentItem{};
                //             contentItem.ItemId = childNode->hash;
                //             contentItem.ShowDetails = false;
                //             contentItem.Label = childNode->fileName.CStr();
                //             contentItem.DragDropType = AssetDragDropType;
                //             contentItem.AcceptPayload = AssetDragDropType;
                //             //contentItem.TooltipText = asset->GetPath().CStr();
                //             contentItem.Texture = fileTexture;
                //
                //             // if (asset->IsModified())
                //             // {
                //             //     contentItem.PreLabel = "*";
                //             // }
                //
                //             if (ImGui::DrawContentItem(contentItem))
                //             {
                //
                //             }
                //
                //         }
                //
                //         // for (AssetHandler* assetHandler : openDirectory->GetChildren())
                //         // {
                //         //     if (DirectoryAssetHandler* directoryInfo = dynamic_cast<DirectoryAssetHandler*>(assetHandler); directoryInfo == nullptr)
                //         //     {
                //         //         ImGui::ContentItemDesc contentItem{};
                //         //         contentItem.ItemId = reinterpret_cast<usize>(assetHandler);
                //         //         contentItem.ShowDetails = true;
                //         //         contentItem.Label = assetHandler->GetName().CStr();
                //         //         contentItem.DetailsDesc = assetHandler->GetDisplayName().CStr();
                //         //         contentItem.DragDropType = AssetDragDropType;
                //         //         contentItem.DragDropPayload = &assetPayload;
                //         //         contentItem.DragDropPayloadSize = sizeof(AssetPayload);
                //         //         contentItem.TooltipText = assetHandler->GetPath().CStr();
                //         //         contentItem.Texture = fileTexture;
                //         //
                //         //         if (assetHandler->IsModified())
                //         //         {
                //         //             contentItem.PreLabel = "*";
                //         //         }
                //         //
                //         //         if (ImGui::DrawContentItem(contentItem))
                //         //         {
                //         //             if (SceneObjectAsset* sceneObjectAsset = dynamic_cast<SceneObjectAsset*>(assetHandler->LoadInstance()))
                //         //             {
                //         //                 Editor::CreateTransaction()->CreateAction<OpenSceneAction>(Editor::GetSceneEditor(), sceneObjectAsset)->Commit();
                //         //             }
                //         //             // else if (node->objectType == GetTypeID<GraphAsset>() || node->objectType == GetTypeID<RenderGraphAsset>())
                //         //             // {
                //         //             //     GraphEditorWindow::OpenGraphWindow(node->rid);
                //         //             // }
                //         //         }
                //         //
                //         //         if (ImGui::ContentItemSelected(contentItem.ItemId))
                //         //         {
                //         //             newItemSelected = assetHandler;
                //         //         }
                //         //
                //         //         if (ImGui::ContentItemRenamed(contentItem.ItemId))
                //         //         {
                //         //             if (ImGui::ContentRenameString() != assetHandler->GetName())
                //         //             {
                //         //                 Editor::CreateTransaction()->CreateAction<RenameAssetAction>(assetHandler, ImGui::ContentRenameString())->Commit();
                //         //             }
                //         //         }
                //         //
                //         //         if (ImGui::ContentItemBeginPayload(contentItem.ItemId))
                //         //         {
                //         //             assetPayload.asset = selectedItem;
                //         //         }
                //         //     }
                //         // }
                //     }
                //     ImGui::EndContentTable();
                //
                //     if (!ImGui::RenamingSelected(CONTENT_TABLE_ID + id) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
                //     {
                //         if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && openDirectory)
                //         {
                //         //    selectedDiretory = dynamic_cast<DirectoryAssetHandler*>(openDirectory->GetParent());
                //         }
                //     }
                // }

              //   if (selectedDiretory)
              //   {
              // //      SetOpenDirectory(selectedDiretory);
              //   }

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

    }

    void ProjectBrowserWindow::RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Project Browser", .action = OpenProjectBrowser});
        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Bottom,
            .createOnInit = true
        });
    }
}
