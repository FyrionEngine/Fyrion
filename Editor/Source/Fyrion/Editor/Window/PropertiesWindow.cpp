

#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"

namespace Fyrion
{

    namespace
    {
        MenuItemContext menuItemContext{};

        void Shutdown()
        {
            menuItemContext = MenuItemContext{};
        }
    }

    struct ProjectBrowserWindow : EditorWindow
    {
        EditorWindowProperties GetProperties() override
        {
            return {
                .dockPosition = DockPosition::Bottom,
                .createOnInit = true
            };
        }

        void Draw(u32 id, bool& open) override
        {
            ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin(id, ICON_FA_LIST " Entity Tree", &open, ImGuiWindowFlags_NoScrollbar);

            ImGui::End();
        }
    };


    void OpenProjectBrowser(VoidPtr userData)
    {
        Editor::OpenWindow(GetTypeID<ProjectBrowserWindow>());
    }

    void InitProjectBrowser()
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Project Browser", .action = OpenProjectBrowser});

        Event::Bind<OnShutdown, Shutdown>();
        Registry::Type<ProjectBrowserWindow, EditorWindow>();
    }
}