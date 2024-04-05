

#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
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
        Registry::Type<ProjectBrowserWindow, EditorWindow>();
    }
}