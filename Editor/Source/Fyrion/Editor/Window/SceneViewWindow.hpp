#pragma once
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    class RenderGraph;
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
        SharedPtr<RenderGraph> renderGraph{};

        static void OpenSceneView(const MenuItemEventData& eventData);
    };
}
