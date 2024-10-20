#include "Editor.hpp"

#include <mutex>

#include "EditorTypes.hpp"
#include "Action/EditorAction.hpp"
#include "Asset/AssetTypes.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/Lib/imgui_internal.h"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Scene/SceneEditor.hpp"
#include "Window/ProjectBrowserWindow.hpp"
#include "Window/PropertiesWindow.hpp"
#include "Window/SceneTreeWindow.hpp"
#include "Window/TextureViewWindow.hpp"

namespace Fyrion
{
    void InitEditorAction();
    void RegisterAssetTypes();
    void AssetEditorInit();
    void ImGuiUpdate();
    void ImGuiShutdown();
    void RegisterFieldRenderers();
    void ShaderManagerInit();
    void ShaderManagerShutdown();

    struct EditorWindowStorage
    {
        TypeID       typeId{};
        FnCast       fnCast{};
        DockPosition dockPosition{};
        bool         createOnInit{};
    };

    struct OpenWindowStorage
    {
        u32           id{};
        EditorWindow* instance{};
        TypeHandler*  typeHandler{};
    };

    namespace
    {
        Array<EditorWindowStorage>   editorWindowStorages{};
        Array<OpenWindowStorage>     openWindows{};
        Array<AssetFile*>            updatedItems{};
        bool                         shouldOpenPopup = false;

        std::mutex                   callsMutex;
        Array<std::function<void()>> calls{};

        MenuItemContext menuContext{};
        bool            dockInitialized = false;
        u32             dockSpaceId{10000};
        u32             centerSpaceId{10000};
        u32             topRightDockId{};
        u32             bottomRightDockId{};
        u32             bottomDockId{};
        u32             leftDockId{};
        u32             idCounter{100000};
        bool            showImGuiDemo = false;

        bool forceClose{};

        Array<SharedPtr<EditorTransaction>> undoActions{};
        Array<SharedPtr<EditorTransaction>> redoActions{};

        SceneEditor sceneEditor{};

        void SaveAll(Span<AssetFile*> assets);

        void Shutdown()
        {
            menuContext = {};

            for (OpenWindowStorage& openWindow : openWindows)
            {
                if (openWindow.instance)
                {
                    openWindow.typeHandler->Destroy(openWindow.instance);
                }
            }

            ImGuiShutdown();
            ShaderManagerShutdown();

            openWindows.Clear();
            openWindows.ShrinkToFit();
            editorWindowStorages.Clear();
            editorWindowStorages.ShrinkToFit();
            idCounter = 100000;

            undoActions.Clear();
            undoActions.ShrinkToFit();

            redoActions.Clear();
            redoActions.ShrinkToFit();

            calls.Clear();
            calls.ShrinkToFit();
        }

        void InitEditor()
        {
            //TODO - this needs to be update on reload.
            TypeHandler*      editorWindow = Registry::FindType<EditorWindow>();
            Span<DerivedType> derivedTypes = editorWindow->GetDerivedTypes();
            for (const DerivedType& derivedType : derivedTypes)
            {
                EditorWindowProperties properties{};
                TypeHandler*           typeHandler = Registry::FindTypeById(derivedType.typeId);
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

            AssetEditorInit();
            AssetEditor::AddPackage("Fyrion", "C:\\dev\\Fyrion\\Fyrion");
            AssetEditor::SetProject("Refactor", "C:\\dev\\Fyrion\\Projects\\Refactor");
        }

        void CloseEngine(const MenuItemEventData& eventData)
        {
            Engine::Shutdown();
        }

        void NewProject(const MenuItemEventData& eventData) {}

        void SaveAll(const MenuItemEventData& eventData)
        {
            AssetEditor::GetUpdatedAssets(updatedItems);
            SaveAll(updatedItems);
            updatedItems.Clear();
        }

        void ShowImGuiDemo(const MenuItemEventData& eventData)
        {
            showImGuiDemo = true;
        }

        void Undo(const MenuItemEventData& eventData)
        {
            SharedPtr<EditorTransaction> action = undoActions.Back();
            action->Rollback();
            redoActions.EmplaceBack(action);
            undoActions.PopBack();
        }

        bool UndoEnabled(const MenuItemEventData& eventData)
        {
            return !undoActions.Empty();
        }

        void Redo(const MenuItemEventData& eventData)
        {
            SharedPtr<EditorTransaction> action = redoActions.Back();
            action->Commit();

            redoActions.PopBack();
            undoActions.EmplaceBack(action);
        }

        bool RedoEnabled(const MenuItemEventData& eventData)
        {
            return !redoActions.Empty();
        }

        void CreateMenuItems()
        {
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File", .priority = 0});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File/New Project", .priority = 0, .action = NewProject});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File/Save All", .priority = 1000, .itemShortcut{.ctrl = true, .presKey = Key::S}, .action = SaveAll});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "File/Exit", .priority = I32_MAX, .itemShortcut{.ctrl = true, .presKey = Key::Q}, .action = CloseEngine});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Edit", .priority = 30});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Edit/Undo", .priority = 10, .itemShortcut{.ctrl = true, .presKey = Key::Z}, .action = Undo, .enable = UndoEnabled});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Edit/Redo", .priority = 20, .itemShortcut{.ctrl = true, .shift = true, .presKey = Key::Z}, .action = Redo, .enable = RedoEnabled});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Build", .priority = 40});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Window", .priority = 50});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Help", .priority = 60});
            Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Dear ImGui Demo", .priority = I32_MAX, .action = ShowImGuiDemo});
        }


        u32 GetDockId(DockPosition dockPosition)
        {
            switch (dockPosition)
            {
                case DockPosition::None: return U32_MAX;
                case DockPosition::Center: return centerSpaceId;
                case DockPosition::Left: return leftDockId;
                case DockPosition::TopRight: return topRightDockId;
                case DockPosition::BottomRight: return bottomRightDockId;
                case DockPosition::Bottom: return bottomDockId;
            }
            return U32_MAX;
        }

        u32 CreateWindow(const EditorWindowStorage& editorWindowStorage, VoidPtr userData)
        {
            TypeHandler* typeHandler = Registry::FindTypeById(editorWindowStorage.typeId);
            u32          windowId = idCounter;

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
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.46f, 0.49f, 0.50f, 0.67f));
            menuContext.ExecuteHotKeys(nullptr, true);
            if (ImGui::BeginMenuBar())
            {
                menuContext.Draw();
                ImGui::EndMenuBar();
            }
            ImGui::PopStyleColor(1);
        }


        void InitDockSpace()
        {
            if (!dockInitialized)
            {
                dockInitialized = true;
                ImGui::DockBuilderReset(dockSpaceId);

                //create default windows
                centerSpaceId = dockSpaceId;
                topRightDockId = ImGui::DockBuilderSplitNode(centerSpaceId, ImGuiDir_Right, 0.15f, nullptr, &centerSpaceId);
                bottomRightDockId = ImGui::DockBuilderSplitNode(topRightDockId, ImGuiDir_Down, 0.50f, nullptr, &topRightDockId);
                bottomDockId = ImGui::DockBuilderSplitNode(centerSpaceId, ImGuiDir_Down, 0.20f, nullptr, &centerSpaceId);
                leftDockId = ImGui::DockBuilderSplitNode(centerSpaceId, ImGuiDir_Left, 0.12f, nullptr, &centerSpaceId);

                for (const auto& windowType : editorWindowStorages)
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

                if (shouldOpenPopup)
                {
                    ImGui::OpenPopup("Save Content");
                    shouldOpenPopup = false;
                }

                bool                   open{true};
                static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;
                ImGuiStyle&            style = ImGui::GetStyle();
                ImGui::SetNextWindowSize({600 * style.ScaleFactor, 400 * style.ScaleFactor}, ImGuiCond_Once);
                ImGui::StyleColor childBg(ImGuiCol_PopupBg, IM_COL32(28, 31, 33, 255));
                if (ImGui::BeginPopupModal("Save Content", &open, ImGuiWindowFlags_NoScrollbar))
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
                                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 200.f * style.ScaleFactor);
                                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None, 400.f * style.ScaleFactor);
                                ImGui::TableHeadersRow();

                                for (AssetFile* assetFile : updatedItems)
                                {
                                    ImGui::TableNextRow();

                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::Text("%s%s", assetFile->fileName.CStr(), assetFile->extension.CStr());
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::Text("%s", "Unknown");
                                }
                                ImGui::EndTable();
                            }

                            ImGui::EndChild();
                        }

                        ImGui::BeginHorizontal("#horizontal-save", ImVec2(width, buttonHeight));

                        ImGui::Spring(1.0f);

                        if (ImGui::Button("Save All"))
                        {
                            SaveAll(updatedItems);
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

        void SaveAll(Span<AssetFile*> assets)
        {
            AssetEditor::SaveAssets(assets);
        }

        void EditorUpdate(f64 deltaTime)
        {
            Array<std::function<void()>> callsMoved;
            {
                std::unique_lock lock(callsMutex);
                callsMoved = Traits::Move(calls);
            }

            for(const auto& func: callsMoved)
            {
                func();
            }

            ImGuiUpdate();

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

            sceneEditor.DoUpdate();

            ProjectUpdate();
        }

        void OnEditorShutdownRequest(bool* canClose)
        {
            if (forceClose) return;

            updatedItems.Clear();
            AssetEditor::GetUpdatedAssets(updatedItems);

            if (!updatedItems.Empty())
            {
                *canClose = false;
                shouldOpenPopup = true;
            }
        }
    }

    void Editor::OpenWindow(TypeID windowType, VoidPtr initUserData)
    {
        for (const EditorWindowStorage& window : editorWindowStorages)
        {
            if (window.typeId == windowType)
            {
                CreateWindow(window, initUserData);
                break;
            }
        }
    }


    EditorTransaction* Editor::CreateTransaction()
    {
        redoActions.Clear();
        return undoActions.EmplaceBack(MakeShared<EditorTransaction>()).Get();
    }

    void Editor::AddMenuItem(const MenuItemCreation& menuItem)
    {
        menuContext.AddMenuItem(menuItem);
    }

    SceneEditor& Editor::GetSceneEditor()
    {
        return sceneEditor;
    }

    String Editor::CreateProject(StringView newProjectPath, StringView projectName)
    {
        String fullProjectPath = Path::Join(newProjectPath, projectName);
        String assetsPath = Path::Join(fullProjectPath, "Assets");
        String dataPath = Path::Join(fullProjectPath, "Data");
        String settingsPath = Path::Join(fullProjectPath, "Settings");
        String projectFilePath = Path::Join(fullProjectPath, projectName, FY_PROJECT_EXTENSION);

        FileSystem::CreateDirectory(assetsPath);
        FileSystem::CreateDirectory(dataPath);
        FileSystem::CreateDirectory(settingsPath);

        // JsonAssetWriter jsonAssetWriter;
        // auto            object = jsonAssetWriter.CreateObject();
        // jsonAssetWriter.WriteString(object, "engineVersion", FY_VERSION);
        //
        // FileSystem::SaveFileAsString(projectFilePath, JsonAssetWriter::Stringify(object));

        return projectFilePath;
    }

    void Editor::ExecuteOnMainThread(std::function<void()> func)
    {
        std::unique_lock lock(callsMutex);
        calls.EmplaceBack(func);
    }

    void Editor::Init(StringView projectFile)
    {
        // if (Path::Extension(projectFile) != FY_PROJECT_EXTENSION)
        // {
        //     return;
        // }

        //projectPath = Path::Parent(projectFile);

        ShaderManagerInit();

        Registry::Type<EditorWindow>();

        RegisterAssetTypes();
        InitEditorAction();
        RegisterFieldRenderers();

        Registry::Type<ProjectBrowserWindow>();
        Registry::Type<TextureViewWindow>();
        Registry::Type<SceneTreeWindow>();
        Registry::Type<PropertiesWindow>();

        Event::Bind<OnInit, &InitEditor>();
        Event::Bind<OnUpdate, &EditorUpdate>();
        Event::Bind<OnShutdown, &Shutdown>();
        Event::Bind<OnShutdownRequest, &OnEditorShutdownRequest>();

        CreateMenuItems();
    }
}
