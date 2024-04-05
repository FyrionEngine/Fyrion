#include "Editor.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/Lib/imgui_internal.h"
#include "EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{

    namespace
    {
//        Array<EditorWindow>         EditorWindows;
//        Array<OpenWindowStorage>    OpenWindows;
//        EventHandler<OnEditorStart> OnEditorStartEvent{};

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

        void Shutdown()
        {
            menuContext = {};
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
            //Project::SaveAll();
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
    }

    void InitProjectBrowser();
    void InitDockSpace();
    void InitProject();
    void InitWorldView();
    void InitEntityTreeWindow();
    void InitPropertiesWindow();


    void DrawOpenWindows()
    {

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

//            for (const auto& windowType: EditorWindows)
//            {
//                auto p = GetDockId(windowType.InitialDockPosition);
//                if (p != U32_MAX)
//                {
//                    ImGui::DockBuilderDockWindow(CreateWindow(windowType, nullptr), p);
//                }
//            }
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

        Event::Bind<OnUpdate, &EditorUpdate>();
        Event::Bind<OnShutdown, &Shutdown>();

        CreateMenuItems();
    }
}
