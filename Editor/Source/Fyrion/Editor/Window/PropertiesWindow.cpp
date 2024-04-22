#include "PropertiesWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Scene/SceneEditor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    void PropertiesWindow::Draw(u32 id, bool& open)
    {
        ImGui::Begin(id, ICON_FA_CIRCLE_INFO " Properties", &open, ImGuiWindowFlags_NoScrollbar);

        SceneObject* object = Editor::GetSceneEditor().GetLastSelectedObject();
        if (object)
        {
            DrawSceneObject(id, object);
        }

        ImGui::End();
    }

    void PropertiesWindow::OpenProperties(VoidPtr userData)
    {
        Editor::OpenWindow<PropertiesWindow>();
    }

    void PropertiesWindow::DrawSceneObject(u32 id, SceneObject* sceneObject)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        bool readOnly = false;

        if (ImGui::BeginTable("#object-table", 2))
        {
            ImGui::BeginDisabled(readOnly);

            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);

            m_StringCache = sceneObject->GetName();
            u32 hash = HashValue(sceneObject->GetAsset());

            if (ImGui::InputText(hash, m_StringCache))
            {
                m_renamingCache = sceneObject->GetName();
                m_renamingFocus = true;
               //propertiesWindow->renamingEntity = entity;
            }

            if (!ImGui::IsItemActive() && m_renamingFocus)
            {
                //WorldController::RenameEntity(propertiesWindow->renamingEntity, propertiesWindow->renamingCache);
                m_renamingFocus = false;
                m_renamingCache.Clear();
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("UUID");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            String uuid = ToString(Repository::GetUUID(sceneObject->GetAsset()));
            ImGui::InputText(hash + 10, uuid, ImGuiInputTextFlags_ReadOnly);
            ImGui::EndDisabled();
            ImGui::EndTable();

        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        f32 width = ImGui::GetContentRegionAvail().x;
        auto size = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;

        ImGui::BeginHorizontal(9999, ImVec2(width, size));

        ImGui::Spring(1.f);
        bool addComponent = false;

        ImGui::BeginDisabled(readOnly);

        if (ImGui::BorderedButton("Add Component", ImVec2(width * 2 / 3, size)))
        {
            addComponent = true;
        }

        ImGui::EndDisabled();

        ImVec2 max = ImGui::GetItemRectMax();
        ImVec2 min = ImGui::GetItemRectMin();

        ImGui::Spring(1.f);

        ImGui::EndHorizontal();

        RID openAsset{};

        // if (entityData.rid)
        // {
        //
        //     ImGui::BeginHorizontal(9999, ImVec2(width, size));
        //     ImGui::Spring(1.f);
        //
        //     if (ImGui::BorderedButton("Open Asset", ImVec2((width * 2) / 3, size)))
        //     {
        //         openAsset = entityData.rid;
        //     }
        //
        //     ImGui::Spring(1.f);
        //     ImGui::EndHorizontal();
        // }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        // draw components

        if (addComponent)
        {
            ImGui::OpenPopup("add-component-popup");
        }

        ImGui::SetNextWindowPos(ImVec2(min.x, max.y + 5));
        auto sizePopup = max.x - min.x;
        ImGui::SetNextWindowSize(ImVec2(sizePopup, 0), ImGuiCond_Appearing);

        auto popupRes = ImGui::BeginPopupMenu("add-component-popup", 0, false);
        if (popupRes)
        {
            ImGui::SetNextItemWidth(sizePopup - style.WindowPadding.x * 2);
            ImGui::SearchInputText(id + 100, m_searchComponentString);
            ImGui::Separator();

            TypeHandler* componentHandler = Registry::FindType<Component>();
            if (componentHandler)
            {
                for (DerivedType& derivedType : componentHandler->GetDerivedTypes())
                {
                    TypeHandler* typeHandler = Registry::FindTypeById(derivedType.typeId);
                    if (typeHandler)
                    {
                        if (ImGui::Selectable(typeHandler->GetSimpleName().CStr()))
                        {
                            Editor::GetSceneEditor().AddComponent(sceneObject, typeHandler);
                        }
                    }
                }
            }
        }
        ImGui::EndPopupMenu(popupRes);

    }

    void PropertiesWindow::RegisterType(NativeTypeHandler<PropertiesWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName="Window/Properties", .action = OpenProperties});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::BottomRight,
            .createOnInit = true
        });
    }

    void InitPropertiesWindow()
    {
        Registry::Type<PropertiesWindow, EditorWindow>();
    }
}
