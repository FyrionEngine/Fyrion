#include "Editor.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/Lib/imgui_internal.h"
#include "EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{

    void InitProjectBrowser();
//    void InitProject();
//    void InitWorldView();
//    void InitEntityTreeWindow();
//    void InitPropertiesWindow();

    struct EditorWindowStorage
    {
        TypeID typeId{};
        FnCast fnCast{};
        DockPosition dockPosition{};
        bool createOnInit{};
    };

    struct OpenWindowStorage
    {
        u32 id{};
        EditorWindow* instance{};
        TypeHandler* typeHandler{};
    };

    namespace
    {
        Array<EditorWindowStorage> editorWindowStorages{};
        Array<OpenWindowStorage> openWindows{};
        Array<RID> updatedItems{};

        MenuItemContext menuContext{};
        bool dockInitialized = false;
        u32  dockSpaceId{10000};
        u32  centerSpaceId{10000};
        u32  topRightDockId{};
        u32  bottomRightDockId{};
        u32  bottomDockId{};
        u32  leftDockId{};
        u32  idCounter{100000};
        bool showImGuiDemo = false;

        bool forceClose{};

        AssetTree assetTree{};

        void SaveAll();

        void Shutdown()
        {
            menuContext = {};
            openWindows.Clear();
            openWindows.ShrinkToFit();
            editorWindowStorages.Clear();
            editorWindowStorages.ShrinkToFit();
            idCounter = 100000;
        }

        void InitEditor()
        {
            //TODO - this needs to be update on reload.
            TypeHandler* editorWindow = Registry::FindType<EditorWindow>();
            Span<DerivedType> derivedTypes = editorWindow->GetDerivedTypes();
            for(const DerivedType& derivedType : derivedTypes)
            {
                EditorWindowProperties properties{};
                TypeHandler* typeHandler = Registry::FindTypeById(derivedType.typeId);
                if (typeHandler)
                {
                    const EditorWindowProperties* editorWindowProperties = typeHandler->GetAttribute<EditorWindowProperties>();
                    if (editorWindowProperties)
                    {
                        properties.createOnInit = editorWindowProperties->createOnInit;
                        properties.dockPosition = editorWindowProperties->dockPosition;
                    }
                }

                editorWindowStorages.EmplaceBack(EditorWindowStorage{
                    .typeId = derivedType.typeId,
                    .fnCast = derivedType.fnCast,
                    .dockPosition = properties.dockPosition,
                    .createOnInit = properties.createOnInit
                });
            }

            //TODO: Create a setting for that.
            Editor::OpenProject(ResourceAssets::GetAssetRootByName("Fyrion"));

            if (Engine::HasArgByName("projectPath"))
            {
                String projectPath = Engine::GetArgByName("projectPath");
                Editor::OpenProject(ResourceAssets::LoadAssetsFromDirectory(Path::Name(projectPath), Path::Join(projectPath, "Assets")));
            }
        }

        void CloseEngine(VoidPtr userData)
        {
            Engine::Shutdown();
        }

        void NewProject(VoidPtr userData)
        {

        }

        void SaveAll(VoidPtr userData)
        {
            SaveAll();
        }

        void ShowImGuiDemo(VoidPtr userData)
        {
            showImGuiDemo = true;
        }

        void CreateMenuItems()
        {
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File", .priority = 0});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File/New Project", .priority = 0, .action = NewProject});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File/Save All", .priority = 1000, .itemShortcut{.ctrl=true, .presKey = Key::S}, .action = SaveAll});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File/Exit", .priority = I32_MAX, .itemShortcut{.ctrl=true, .presKey = Key::Q}, .action = CloseEngine});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Edit", .priority = 30});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Build", .priority = 40});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Window", .priority = 50});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Help", .priority = 60});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Dear ImGui Demo", .priority = I32_MAX, .action=ShowImGuiDemo});
        }


        u32 GetDockId(DockPosition dockPosition)
        {
            switch (dockPosition)
            {
                case DockPosition::None: return U32_MAX;
                case DockPosition::Center: return centerSpaceId;
                case DockPosition::Left: return leftDockId;
                case DockPosition::TopRight:return topRightDockId;
                case DockPosition::BottomRight: return bottomRightDockId;
                case DockPosition::Bottom: return bottomDockId;
            }
            return U32_MAX;
        }

        u32 CreateWindow(const EditorWindowStorage& editorWindowStorage, VoidPtr userData)
        {
            TypeHandler* typeHandler = Registry::FindTypeById(editorWindowStorage.typeId);
            u32 windowId = idCounter;

            OpenWindowStorage openWindowStorage = OpenWindowStorage{
                .id = windowId,
                .instance = typeHandler->Cast<EditorWindow>(typeHandler->NewInstance()),
                .typeHandler = typeHandler
            };

            openWindowStorage.instance->Init(openWindowStorage.id, userData);

            openWindows.EmplaceBack(openWindowStorage);
            idCounter = idCounter + 1000;

            auto p = GetDockId(editorWindowStorage.dockPosition);
            if (p != U32_MAX)
            {
                ImGui::DockBuilderDockWindow(windowId, p);
            }
            return windowId;
        }

        void DrawOpenWindows()
        {
            for (u32 i = 0; i < openWindows.Size(); ++i)
            {
                OpenWindowStorage& openWindowStorage = openWindows[i];
                bool open = true;
                openWindowStorage.instance->Draw(openWindowStorage.id, open);
                if (!open)
                {
                    openWindowStorage.typeHandler->Destroy(openWindowStorage.instance);
                    openWindows.Erase(openWindows.begin() + i, openWindows.begin() + i + 1);
                }
            }
        }

        void DrawMenu()
        {
            menuContext.ExecuteHotKeys();
            if (ImGui::BeginMenuBar())
            {
                menuContext.Draw();
                ImGui::EndMenuBar();
            }
        }


        void InitDockSpace()
        {
            if (!dockInitialized)
            {
                dockInitialized = true;
                ImGui::DockBuilderReset(dockSpaceId);

                //create default windows
                centerSpaceId     = dockSpaceId;
                topRightDockId    = ImGui::DockBuilderSplitNode(centerSpaceId, ImGuiDir_Right, 0.15f, nullptr, &centerSpaceId);
                bottomRightDockId = ImGui::DockBuilderSplitNode(topRightDockId, ImGuiDir_Down, 0.50f, nullptr, &topRightDockId);
                bottomDockId      = ImGui::DockBuilderSplitNode(centerSpaceId, ImGuiDir_Down, 0.20f, nullptr, &centerSpaceId);
                leftDockId        = ImGui::DockBuilderSplitNode(centerSpaceId, ImGuiDir_Left, 0.12f, nullptr, &centerSpaceId);

                for (const auto& windowType: editorWindowStorages)
                {
                    if (windowType.createOnInit)
                    {
                        CreateWindow(windowType, nullptr);
                    }
                }
            }
        }

        void ProjectUpdate()
        {
            if (!updatedItems.Empty())
            {
                bool open{true};
                static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;
                ImGuiStyle& style = ImGui::GetStyle();
                ImGui::SetNextWindowSize({600 * style.ScaleFactor, 400 * style.ScaleFactor}, ImGuiCond_Once);
                ImGui::StyleColor childBg(ImGuiCol_PopupBg, IM_COL32(28, 31, 33, 255));
                if(ImGui::BeginPopupModal("Save Content", &open, ImGuiWindowFlags_NoScrollbar))
                {
                    ImGui::Text("Pending items to save");
                    {
                        ImGui::StyleColor tableBorderStyleColor(ImGuiCol_TableBorderLight, IM_COL32(0, 0, 0, 0));
                        ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));

                        f32 width = ImGui::GetContentRegionAvail().x - 5;
                        f32 height = ImGui::GetContentRegionAvail().y;
                        f32 buttonHeight = 25 * style.ScaleFactor;

                        if (ImGui::BeginChild(455343, ImVec2(width, height - buttonHeight), false))
                        {
                            if (ImGui::BeginTable("table-pending-to-save", 3, flags))
                            {
                                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 100.f * style.ScaleFactor);
                                ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_None, 300.f * style.ScaleFactor);
                                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None, 200.f * style.ScaleFactor);
                                ImGui::TableHeadersRow();

                                for(const RID& rid: updatedItems)
                                {
                                    AssetNode* node = assetTree.GetNode(rid);
                                    if (node == nullptr) continue;

                                    ImGui::TableNextRow();

                                    ImVec4 color = style.Colors[ImGuiCol_Text];
                                    if (!node->active)
                                    {
                                        color = ImVec4{180.f/255.f, 85.f/255.f, 85.f/255.f, 255};
                                    }

                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::TextColored(color, "%s", node->name.CStr());
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::TextColored(color, "%s", node->path.CStr());
                                    ImGui::TableSetColumnIndex(2);

                                    if (node->type == GetTypeID<AssetDirectory>())
                                    {
                                        ImGui::TextColored(color, "%s", "Directory");
                                    }
                                    else
                                    {
                                        ImGui::TextColored(color, "%s", "Not Defined");
                                    }

                                }
                                ImGui::EndTable();
                            }

                            ImGui::EndChild();
                        }

                        ImGui::BeginHorizontal("#jitrjirt", ImVec2(width, buttonHeight));

                        ImGui::Spring(1.0f);

                        if (ImGui::Button("Save All"))
                        {
                            SaveAll();
                            forceClose = true;
                            Engine::Shutdown();
                        }

                        if (ImGui::Button("Don't Save"))
                        {
                            forceClose = true;
                            Engine::Shutdown();
                        }

                        if (ImGui::Button("Cancel"))
                        {
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndHorizontal();

                    }
                    ImGui::EndPopup();
                }
                else if (!updatedItems.Empty())
                {
                    updatedItems.Clear();
                }
            }
        }

        void SaveAll()
        {
            for (RID assetRoot: assetTree.GetAssetRoots())
            {
                ResourceAssets::SaveAssetsToDirectory(assetRoot, ResourceAssets::GetAbsolutePath(assetRoot));
            }
            assetTree.MarkDirty();
        }

        void EditorUpdate(f64 deltaTime)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::CreateDockSpace(dockSpaceId);
            InitDockSpace();
            DrawOpenWindows();

            if (showImGuiDemo)
            {
                ImGui::ShowDemoWindow(&showImGuiDemo);
            }

            DrawMenu();
            ImGui::End();

            ProjectUpdate();
        }

        void EditorEndFrame()
        {
            assetTree.Update();
        }

        void OnEditorShutdownRequest(bool* canClose)
        {
            if (forceClose) return;

            assetTree.GetUpdated(updatedItems);
            if (!updatedItems.Empty())
            {
                ImGui::OpenPopup("Save Content");
                *canClose = false;
            }
        }
    }

    void Editor::OpenWindow(TypeID windowType, VoidPtr initUserData)
    {
        for (const EditorWindowStorage& window : editorWindowStorages)
        {
            if (window.typeId== windowType)
            {
                CreateWindow(window, initUserData);
                break;
            }
        }
    }

    void Editor::OpenProject(RID rid)
    {
        if (rid)
        {
            assetTree.AddAssetRoot(rid);
        }
    }

    void Editor::AddMenuItem(const MenuItemCreation& menuItem)
    {
        menuContext.AddMenuItem(menuItem);
    }

    void Editor::Init()
    {
        Registry::Type<EditorWindow>();

        InitProjectBrowser();

        Event::Bind<OnInit , &InitEditor>();
        Event::Bind<OnUpdate, &EditorUpdate>();
        Event::Bind<OnEndFrame , &EditorEndFrame>();
        Event::Bind<OnShutdown, &Shutdown>();
        Event::Bind<OnShutdownRequest, &OnEditorShutdownRequest>();

        CreateMenuItems();
    }

    AssetTree& Editor::GetAssetTree()
    {
        return assetTree;
    }
}
