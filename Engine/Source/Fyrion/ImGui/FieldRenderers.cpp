#include "ImGui.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Lib/imgui_internal.h"

namespace Fyrion
{
    void DrawVecField(const String compName, const char* fieldName, float& value, bool* hasChanged, u32 color = 0, f32 speed = 0.005f)
    {
        ImGui::TableNextColumn();
        String id = "##" + compName + "vec3" + fieldName;
        ImGui::BeginHorizontal(ImHashStr(id.CStr()));
        ImGui::Text("%s", fieldName);
        ImGui::Spring();
        ImGui::SetNextItemWidth(-1);
        if (color != 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, color);
        }
        if (ImGui::InputFloat(id.CStr(), &value))
        {
            if (hasChanged)
            {
                *hasChanged = true;
            }
        }

        if (color != 0)
        {
            ImGui::PopStyleColor();
        }

        ImGui::EndHorizontal();
    }

    void Vec3Renderer(FieldHandler* fieldHandler, VoidPtr value, bool* hasChanged)
    {
        f32    speed = 0.005f;
        String compName = ToString(reinterpret_cast<usize>(value));
        Vec3&  vec3 = *static_cast<Vec3*>(value);
        if (ImGui::BeginTable("##vec3-table", 3))
        {
            DrawVecField(compName, "X", vec3.x, hasChanged, IM_COL32(138, 46, 61, 255), speed);
            DrawVecField(compName, "Y", vec3.y, hasChanged, IM_COL32(87, 121, 26, 255), speed);
            DrawVecField(compName, "Z", vec3.z, hasChanged, IM_COL32(43, 86, 138, 255), speed);
            ImGui::EndTable();
        }
    }


    void QuatRenderer(FieldHandler* fieldHandler, VoidPtr value, bool* hasChanged)
    {
        static int rotationMode = 0;

        f32    speed = 0.005f;
        String compName = ToString(reinterpret_cast<usize>(value));
        Quat&  quat = *static_cast<Quat*>(value);

        if (rotationMode == 0)
        {
            Vec3 euler = Math::Degrees(Math::EulerAngles(quat));
            bool vecHasChanged = false;

            if (ImGui::BeginTable("##vec3-table", 3))
            {
                DrawVecField(compName, "X", euler.x, &vecHasChanged, IM_COL32(138, 46, 61, 255), speed);
                DrawVecField(compName, "Y", euler.y, &vecHasChanged, IM_COL32(87, 121, 26, 255), speed);
                DrawVecField(compName, "Z", euler.z, &vecHasChanged, IM_COL32(43, 86, 138, 255), speed);
                ImGui::EndTable();
            }

            if (vecHasChanged)
            {
                *static_cast<Quat*>(value) = {Math::Radians(euler)};
                if (hasChanged) *hasChanged = true;
            }
        }
        else
        {
            if (ImGui::BeginTable("##quat-table", 4))
            {
                DrawVecField(compName, "X", quat.x, hasChanged, IM_COL32(138, 46, 61, 255), speed);
                DrawVecField(compName, "Y", quat.y, hasChanged, IM_COL32(87, 121, 26, 255), speed);
                DrawVecField(compName, "Z", quat.z, hasChanged, IM_COL32(43, 86, 138, 255), speed);
                DrawVecField(compName, "W", quat.w, hasChanged, IM_COL32(84, 74, 119, 255), speed);
                ImGui::EndTable();
            }
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            auto currentTable = ImGui::GetCurrentTable();
            if (currentTable && ImGui::TableGetHoveredRow() == currentTable->CurrentRow)
            {
                ImGui::OpenPopup("open-rotation-mode-popup");
            }
        }

        bool popupOpenSettings = ImGui::BeginPopupMenu("open-rotation-mode-popup", 0, false);
        if (popupOpenSettings)
        {
            if (ImGui::MenuItem("Euler", "", rotationMode == 0))
            {
                rotationMode = 0;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Quaternion", "", rotationMode == 1))
            {
                rotationMode = 1;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopupMenu(popupOpenSettings);
    }

    void RegisterFieldRenderers()
    {
        ImGui::AddFieldRenderer(GetTypeID<Vec3>(), Vec3Renderer);
        ImGui::AddFieldRenderer(GetTypeID<Quat>(), QuatRenderer);
    }
}
