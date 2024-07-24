#include "SceneViewWindow.hpp"

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Action/SceneEditorAction.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/ImGui/Lib/ImGuizmo.h"
#include "Fyrion/IO/Input.hpp"
#include "Fyrion/Scene/Components/TransformComponent.hpp"

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

            if (!movingScene)
            {
                movingScene = !windowStartedSimulation && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && Input::IsMouseDown(MouseButton::Right);
            }

            if (movingScene)
            {
                bool rightDown = Input::IsMouseDown(MouseButton::Right);
                freeViewCamera.SetActive(rightDown);
                movingScene = rightDown;
            }

            freeViewCamera.Process(Engine::DeltaTime());

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

            CameraData cameraData = CameraData{
                .view = freeViewCamera.GetView(),
                .projection = Math::Perspective(Math::Radians(60.f),
                                                (f32)extent.width / (f32)extent.height,
                                                0.1,
                                                1000),
                .lastViewProj = Mat4{1.0},
                .viewPos = freeViewCamera.GetPosition()
            };

            renderGraph->SetCameraData(cameraData);

            if (open)
            {
                ImGui::DrawImage(renderGraph->GetColorOutput(), bb);
            }

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(cursor.x, cursor.y, size.x, size.y);

            for (auto it : sceneEditor.GetSelectedObjects())
            {
                SceneObject* object = reinterpret_cast<SceneObject*>(it.first);
                if (TransformComponent* transformComponent = object->GetComponent<TransformComponent>())
                {
                    Mat4 worldMatrix = transformComponent->GetWorldTransform();

                    static float snap[3] = {0.0f, 0.0f, 0.0f};

                    ImGuizmo::Manipulate(&cameraData.view[0][0],
                                         &cameraData.projection[0][0],
                                         static_cast<ImGuizmo::OPERATION>(guizmoOperation),
                                         ImGuizmo::LOCAL,
                                         &worldMatrix[0][0],
                                         nullptr,
                                         snap);

                    if (ImGuizmo::IsUsing())
                    {
                        if (!usingGuizmo)
                        {
                            usingGuizmo = true;

                            gizmoInitialTransform = transformComponent->GetTransform();

                            gizmoTransaction = Editor::CreateTransaction();

                            if (object->GetPrototype() != nullptr && !object->IsComponentOverride(transformComponent))
                            {
                                gizmoTransaction->CreateAction<OverridePrototypeComponentAction>(sceneEditor, object, static_cast<Component*>(transformComponent))->Commit();
                            }
                        }

                        if (TransformComponent* parentTransform = object->GetParent()->GetComponent<TransformComponent>())
                        {
                            worldMatrix = Math::Inverse(parentTransform->GetWorldTransform()) * worldMatrix;
                        }

                        Vec3 position, rotation, scale;
                        Math::Decompose(worldMatrix, position, rotation, scale);
                        auto deltaRotation = rotation - Math::EulerAngles(transformComponent->GetRotation());

                        transformComponent->SetTransform(position, Math::EulerAngles(transformComponent->GetRotation()) + deltaRotation, scale);

                        ImGui::ClearDrawType(reinterpret_cast<usize>(transformComponent));
                    }
                    else if (usingGuizmo)
                    {
                        gizmoTransaction->CreateAction<MoveTransformObjectAction>(sceneEditor, object, transformComponent, gizmoInitialTransform)->Commit();
                        usingGuizmo = false;
                    }
                }
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
