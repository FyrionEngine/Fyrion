#pragma once
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    struct MenuItemEventData;
    class SceneEditor;

    class SceneViewWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);

        SceneViewWindow();

        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<SceneViewWindow>& type);

    private:
        SceneEditor&           sceneEditor;
        u32                    guizmoOperation{};
        bool                   windowStartedSimulation{};
        bool                   movingScene{};

        static void OpenSceneView(const MenuItemEventData& eventData);
    };
}
