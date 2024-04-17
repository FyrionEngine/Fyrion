#pragma once

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Graphics/ViewportRenderer.hpp"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    class WorldViewWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<WorldViewWindow>& type);
    private:
        u32              m_guizmoOperation{ImGuizmo::TRANSLATE};
        ViewportRenderer m_viewportRenderer{};
        bool             m_movingScene{};

        static void OpenWorldView(VoidPtr userData);
    };
}
