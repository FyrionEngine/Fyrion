#include "SceneViewWindow.hpp"

#include "Fyrion/Engine.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/Lib/ImGuizmo.h"
#include "Fyrion/IO/Input.hpp"

namespace Fyrion
{
    SceneViewWindow::SceneViewWindow() : sceneEditor(Editor::GetSceneEditor()), guizmoOperation(ImGuizmo::TRANSLATE) {}

    void SceneViewWindow::Draw(u32 id, bool& open)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        auto&            style = ImGui::GetStyle();
        ImGui::StyleVar  windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
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

                if (ImGui::SelectionButton(ICON_FA_ARROW_POINTER, guizmoOperation == 0, buttonSize)
                    || (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Q))))
                {
                    guizmoOperation = 0;
                }

                if (ImGui::SelectionButton(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, guizmoOperation == ImGuizmo::TRANSLATE, buttonSize)
                    || (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_W))))
                {
                    guizmoOperation = ImGuizmo::TRANSLATE;
                }

                if (ImGui::SelectionButton(ICON_FA_ROTATE, guizmoOperation == ImGuizmo::ROTATE, buttonSize)
                    || (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_E))))
                {
                    guizmoOperation = ImGuizmo::ROTATE;
                }
                if (ImGui::SelectionButton(ICON_FA_EXPAND, guizmoOperation == ImGuizmo::SCALE, buttonSize)
                    || (canChangeGuizmo && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_R))))
                {
                    guizmoOperation = ImGuizmo::SCALE;
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

                bool isSimulating = sceneEditor.IsSimulating();

                if (!isSimulating)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(139, 194, 74, 255));
                }

                if (windowStartedSimulation && !isSimulating)
                {
                    windowStartedSimulation = false;
                }

                ImGui::BeginDisabled(isSimulating);

                if (ImGui::Button(ICON_FA_PLAY, buttonSize))
                {
                    sceneEditor.StartSimulation();
                    windowStartedSimulation = true;
                }

                ImGui::EndDisabled();

                if (!isSimulating)
                {
                    ImGui::PopStyleColor();
                }

                ImGui::BeginDisabled(!sceneEditor.IsSimulating() || !windowStartedSimulation);

                if (isSimulating)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(199, 84, 80, 255));
                }

                if (ImGui::Button(ICON_FA_STOP, buttonSize))
                {
                    sceneEditor.StopSimulation();
                    windowStartedSimulation = false;
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

            if (movingScene)
            {
                ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
            }
            else
            {
                ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }

            movingScene = !windowStartedSimulation && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseDown(ImGuiMouseButton_Right);

            if (movingScene)
            {
                freeViewCamera.SetActive(Input::IsMouseDown(MouseButton::Right));
                freeViewCamera.Process(Engine::DeltaTime());
                movingScene = Input::IsMouseDown(MouseButton::Right);
            }

            auto diffCursor = cursor - initCursor;
            size -= diffCursor;
            Rect bb{(i32)cursor.x, (i32)cursor.y, u32(cursor.x + size.x), u32(cursor.y + size.y)};

            Extent extent = {static_cast<u32>(size.x), static_cast<u32>(size.y)};


            if (!renderGraph)
            {
                renderGraph = MakeShared<RenderGraph>(extent, AssetDatabase::FindByPath<RenderGraphAsset>("Fyrion://DefaultRenderGraph.fy_asset"));
            }

            if (extent != renderGraph->GetViewportExtent())
            {
                renderGraph->Resize(extent);
            }

            renderGraph->SetCameraData(CameraData{
                .view = freeViewCamera.GetView(),
                .projection = Math::Perspective(Math::Radians(60.f),
                                                (f32)extent.width / (f32)extent.height,
                                                0.1,
                                                1000),
                .lastViewProj = Mat4{1.0},
                .viewPos = freeViewCamera.GetPosition()
            });

            if (open)
            {
                ImGui::DrawImage(renderGraph->GetColorOutput(), bb);
            }
        }
        ImGui::End();
    }

    void SceneViewWindow::OpenSceneView(const MenuItemEventData& eventData)
    {
        Editor::OpenWindow<SceneViewWindow>();
    }

    void SceneViewWindow::RegisterType(NativeTypeHandler<SceneViewWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Scene Viewport", .action = OpenSceneView});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Center,
            .createOnInit = true
        });
    }

    void InitSceneViewWindow()
    {
        Registry::Type<SceneViewWindow>();
    }
}
