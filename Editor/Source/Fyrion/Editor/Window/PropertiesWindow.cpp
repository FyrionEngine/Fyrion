

#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Engine.hpp"

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

        }
    };



    void InitProjectBrowser()
    {
        Event::Bind<OnShutdown, Shutdown>();
        Registry::Type<ProjectBrowserWindow, EditorWindow>();
    }
}