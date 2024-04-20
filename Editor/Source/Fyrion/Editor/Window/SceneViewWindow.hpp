#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    class SceneEditor;

    class SceneViewWindow : public EditorWindow
    {
    public:
        SceneViewWindow();

        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<SceneViewWindow>& type);
    private:
        SceneEditor& m_sceneEditor;
        u32 m_guizmoOperation{};
        bool m_windowStartedSimulation{};

        static void OpenSceneView(VoidPtr userData);
    };
}
