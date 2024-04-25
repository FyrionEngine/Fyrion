#include "SceneViewWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Scene/SceneEditor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/Lib/ImGuizmo.h"

namespace Fyrion
{
    SceneViewWindow::SceneViewWindow() : m_sceneEditor(Editor::GetSceneEditor()),  m_guizmoOperation(ImGuizmo::TRANSLATE)
    {
    }

    void SceneViewWindow::Draw(u32 id, bool& open)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        auto& style = ImGui::GetStyle();
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGuizmo::IsUsing() || ImGuizmo::IsOver())
        {
            flags |= ImGuiWindowFlags_NoMove;
        }

        ImGui::Begin(id, ICON_FA_BORDER_ALL " Scene Viewport", &open, flags);
        {
	        bool   moving = ImGui::IsMouseDown(ImGuiMouseButton_Right);
	        bool   canChangeGuizmo = !moving && !ImGui::GetIO().WantCaptureKeyboard;
	        bool   hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
	        bool   openPopup = false;
	        ImVec2 size = ImGui::GetWindowSize();
	        ImVec2 initCursor = ImGui::GetCursorScreenPos();
	        ImVec2 buttonSize = ImVec2(25 * style.ScaleFactor, 22 * style.ScaleFactor);
	        ImVec2 cursor{};

            {
				ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(style.ScaleFactor * 2, style.ScaleFactor * 2));
				ImGui::StyleVar space(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1));
				ImGui::BeginChild(id + 1000, ImVec2(0, buttonSize.y + (5 * style.ScaleFactor)), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
				//ImGui::StyleVar itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::BeginHorizontal("horizontal-sceneview-top", ImVec2(ImGui::GetContentRegionAvail().x, buttonSize.y));

				if (ImGui::SelectionButton(ICON_FA_ARROW_POINTER, m_guizmoOperation == 0, buttonSize)
					|| (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Q))))
				{
					m_guizmoOperation = 0;
				}

				if (ImGui::SelectionButton(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, m_guizmoOperation == ImGuizmo::TRANSLATE, buttonSize)
					|| (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_W))))
				{
					m_guizmoOperation = ImGuizmo::TRANSLATE;
				}

				if (ImGui::SelectionButton(ICON_FA_ROTATE, m_guizmoOperation == ImGuizmo::ROTATE, buttonSize)
					|| (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_E))))
				{
					m_guizmoOperation = ImGuizmo::ROTATE;
				}
				if (ImGui::SelectionButton(ICON_FA_EXPAND, m_guizmoOperation == ImGuizmo::SCALE, buttonSize)
					|| (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_R))))
				{
					m_guizmoOperation = ImGuizmo::SCALE;
				}

				if (ImGui::Button(ICON_FA_ELLIPSIS, buttonSize))
				{
					openPopup = true;
				}

//				auto operation = m_guizmoOperation;

//                        if (entityManager.isAlive(sceneWindowIt.second.selectedEntity)) {
//                            bool isReadOnly = entityManager.isReadOnly(sceneWindow.selectedEntity);
//                            if (isReadOnly) {
//                                operation = 0;
//                            }
//                        }

				ImGui::Spring(1.f);

				bool isSimulating = m_sceneEditor.IsSimulating();

				if (!isSimulating)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(139, 194, 74, 255));
				}

				if (m_windowStartedSimulation && !isSimulating)
				{
					m_windowStartedSimulation = false;
				}

				ImGui::BeginDisabled(isSimulating);

				if (ImGui::Button(ICON_FA_PLAY, buttonSize))
				{
					//WorldController::StartSimulation(world);
					m_windowStartedSimulation = true;
				}

				ImGui::EndDisabled();

				if (!isSimulating)
				{
					ImGui::PopStyleColor();
				}

				ImGui::BeginDisabled(!m_sceneEditor.IsSimulating() || !m_windowStartedSimulation);

				if (isSimulating)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(199, 84, 80, 255));
				}

				if (ImGui::Button(ICON_FA_STOP, buttonSize))
				{
					//WorldController::StopSimulation(world);
					m_windowStartedSimulation = false;
				}

				if (isSimulating)
				{
					ImGui::PopStyleColor();
				}

				ImGui::EndDisabled();

				ImGui::Spring(1.f);

				ImGui::EndHorizontal();

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);

				cursor.y = ImGui::GetCursorScreenPos().y;
				ImGui::EndChild();
				cursor.x = ImGui::GetCursorScreenPos().x;
            }

        	if (m_movingScene)
        	{
        		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        	}
        	else
        	{
        		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        	}

        	m_movingScene = !m_windowStartedSimulation && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseDown(ImGuiMouseButton_Right);

        	auto diffCursor = cursor - initCursor;
        	size -= diffCursor;
        	Rect bb{(i32)cursor.x, (i32)cursor.y, u32(cursor.x + size.x), u32(cursor.y + size.y)};
	        m_viewportRenderer.SetSize(Extent{(u32)size.x, (u32)size.y});

        	if (open)
        	{
        		//ImGui::DrawImage(m_viewportRenderer.GetColorAttachmentOutput(), bb);
        	}

        }
        ImGui::End();
    }

    void SceneViewWindow::OpenSceneView(VoidPtr userData)
    {
        Editor::OpenWindow<SceneViewWindow>();
    }

    void SceneViewWindow::RegisterType(NativeTypeHandler<SceneViewWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Scene Viewport", .action = OpenSceneView});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Center,
            .createOnInit = true
        });
    }

    void InitSceneViewWindow()
    {
        Registry::Type<SceneViewWindow, EditorWindow>();
    }


}
