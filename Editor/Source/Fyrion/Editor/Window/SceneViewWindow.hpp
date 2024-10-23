#pragma once
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Editor/Action/EditorAction.hpp"
#include "Fyrion/Editor/Scene/SceneEditor.hpp"
#include "Fyrion/Graphics/FreeViewCamera.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/RenderPipeline.hpp"

namespace Fyrion
{
    class SceneViewWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);

        SceneViewWindow();
        ~SceneViewWindow() override;

        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<SceneViewWindow>& type);

    private:
        SceneEditor&    sceneEditor;
        u32             guizmoOperation{1};
        bool            windowStartedSimulation{};
        bool            movingScene{};
        RenderGraph*    renderGraph{};
        RenderPipeline* renderPipeline{};
        FreeViewCamera  freeViewCamera{};

        bool               usingGuizmo{};
        Transform          gizmoInitialTransform = {};
        EditorTransaction* gizmoTransaction = nullptr;

        static void OpenSceneView(const MenuItemEventData& eventData);
    };
}
