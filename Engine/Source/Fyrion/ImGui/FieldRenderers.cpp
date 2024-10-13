#include "IconsFontAwesome6.h"
#include "ImGui.hpp"
#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Color.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Lib/imgui_internal.h"

namespace Fyrion
{
    void DrawRID(ImGui::DrawTypeContent* context, FieldHandler* fieldHandler, VoidPtr value, bool* hasChanged)
    {
#if 0
        const ResourceReference* resourceReference = fieldHandler ? fieldHandler->GetAttribute<ResourceReference>() : nullptr;
        ImGuiID id = ImHashData(value, sizeof(VoidPtr));

        RID rid = *static_cast<RID*>(value);
        RID parent = Repository::GetParent(rid);

        String name;
        if (parent &&  Repository::GetResourceTypeId(Repository::GetResourceType(parent)) == GetTypeID<Asset>())
        {
            ResourceObject assetObject = Repository::Read(parent);
            name = Traits::Move(assetObject[Asset::Name].Value<String>());
        }


        ImGui::SetNextItemWidth(-22 * ImGui::GetStyle().ScaleFactor);

        ImGui::InputText(id, name, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine(0, 0);
        ImGui::PushID(id);
        auto size = ImGui::GetItemRectSize();
        if (ImGui::Button(ICON_FA_CIRCLE_DOT, ImVec2{size.y, size.y}))
        {
            if (resourceReference)
            {
                context->showResourceSelection = true;
                context->resourceTypeSelection = resourceReference->resourceType;
                context->fieldShowSelection = fieldHandler;
            }
        }
        ImGui::PopID();

        if (rid && resourceReference && resourceReference->resourceType == GetTypeID<GraphAsset>())
        {
            ImGui::EndTable();


            ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
            ImGui::StyleVar childRound(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);
            if (ImGui::BeginChild("###child", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_AutoResizeY))
            {
                ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(35, 36, 38, 255));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(35, 36, 38, 255));
                ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

                String title = name + " (Inputs)";
                bool open = ImGui::CollapsingHeader(title.CStr());
                ImGui::PopStyleColor(5);
                if (open)
                {
                    ImGui::Indent();

                    if (ImGui::BeginTable("##child-table", 2))
                    {
                        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.4f);
                        ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);

                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();

                        ResourceObject graphObject = Repository::Read(rid);

                        if(graphObject)
                        {
                            Array<RID> nodes = graphObject.GetSubObjectSetAsArray(GraphAsset::Nodes);

                            for (RID node : nodes)
                            {
                                ResourceObject nodeObject = Repository::Read(node);
                                Array<RID> inputs = nodeObject.GetSubObjectSetAsArray(GraphNodeAsset::InputValues);
                                for(RID input: inputs)
                                {
                                    ResourceObject inputObject = Repository::Read(input);
                                    if (inputObject[GraphNodeValue::PublicValue].Value<bool>())
                                    {
                                        String name = inputObject[GraphNodeValue::Name].Value<String>();
                                        RID valueRid = inputObject[GraphNodeValue::Value].Value<RID>();

                                        ImGui::Text(name.CStr());
                                        ImGui::TableNextColumn();

                                        ImGui::DrawType(ImGui::DrawTypeDesc{
                                            .itemId = input.id,
                                            .rid = input,
                                            .typeHandler = Registry::FindTypeByName(inputObject[GraphNodeValue::Type].Value<String>()),
                                            .instance = valueRid ? Repository::ReadData(input) : nullptr,
                                            .userData = nullptr,
                                            .callback = [](ImGui::DrawTypeDesc& desc, ConstPtr newValue)
                                            {
                                                int a= 0;
                                                //TODO - need to create a instance.
                                            }
                                        });

                                        ImGui::TableNextColumn();
                                    }
                                }
                            }
                        }

                        ImGui::EndTable();
                    }
                    ImGui::Unindent();
                }
                ImGui::EndChild();
            }

            ImGui::BeginTable("##component-table", 2);
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
        }
#endif
    }

    bool DrawArrayField(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.apiId != GetTypeID<ArrayApi>()) return false;

        ArrayApi arrayApi{};
        typeInfo.extractApi(&arrayApi);

        bool canAdd = true;
        bool canRemove = true;

        if (const UIArrayProperty* uiArrayProperty = context->activeFieldHandler != nullptr ? context->activeFieldHandler->GetAttribute<UIArrayProperty>() : nullptr)
        {
            canAdd = uiArrayProperty->canAdd;
            canRemove = uiArrayProperty->canRemove;
        }

        ImGui::BeginDisabled(!canAdd);

        if (ImGui::Button(ICON_FA_PLUS))
        {
            arrayApi.pushNew(value);
            if (hasChanged)
            {
                *hasChanged = true;
            }
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(!canRemove);

        if (ImGui::Button(ICON_FA_MINUS))
        {
            arrayApi.popBack(value);
            if (hasChanged)
            {
                *hasChanged = true;
            }
        }

        ImGui::EndDisabled();

        usize size = arrayApi.size(value);
        for (int i = 0; i < size; ++i)
        {
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();

            for (ImGui::FieldRendererFn render : ImGui::GetFieldRenderers())
            {
                render(context, arrayApi.getTypeInfo(), arrayApi.get(value, i), hasChanged);
            }
        }

        return true;
    }

    struct DrawAssetFieldUserData
    {
        ImGui::DrawTypeContent* context;
        TypeInfo typeInfo;
        VoidPtr fieldValue;
    };

    // bool DrawAssetField(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    // {
    //     if (typeInfo.apiId != GetTypeID<AssetApi>()) return false;
    //
    //     AssetApi assetApi{};
    //     typeInfo.extractApi(&assetApi);
    //
    //     //TODO check if that's a pointer
    //     Asset* asset = assetApi.castAsset(value);
    //     String name = asset ? asset->GetHandler()->GetName() : "";
    //
    //     ImGui::SetNextItemWidth(-22 * ImGui::GetStyle().ScaleFactor);
    //
    //     ImGui::PushID(context->ReserveID());
    //
    //     ImGui::InputText(context->ReserveID(), name, ImGuiInputTextFlags_ReadOnly);
    //     ImGui::SameLine(0, 0);
    //     auto size = ImGui::GetItemRectSize();
    //     if (ImGui::Button(ICON_FA_CIRCLE_DOT, ImVec2{size.y, size.y}))
    //     {
    //         static DrawAssetFieldUserData data{};
    //         data.context = context;
    //         data.typeInfo = typeInfo;
    //         data.fieldValue = value;
    //
    //         ImGui::ShowAssetSelector(typeInfo.typeId, &data, [](VoidPtr userData, Asset* asset)
    //         {
    //             DrawAssetFieldUserData* data = static_cast<DrawAssetFieldUserData*>(userData);
    //
    //             AssetApi assetApi{};
    //             data->typeInfo.extractApi(&assetApi);
    //             assetApi.setAsset(data->fieldValue, asset);
    //             data->context->hasChanged = true;
    //         });
    //     }
    //     ImGui::PopID();
    //
    //     return true;
    // }


    void DrawVecField(const char* fieldName, float& value, bool* hasChanged, u32 color = 0, f32 speed = 0.005f)
    {
        ImGui::TableNextColumn();

        char buffer[20]{};
        sprintf(buffer,"##%llu", reinterpret_cast<usize>(&value));

        ImGui::BeginHorizontal(ImHashStr(buffer));
        ImGui::Text("%s", fieldName);
        ImGui::Spring();
        ImGui::SetNextItemWidth(-1);
        if (color != 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, color);
        }
        if (ImGui::InputFloat(buffer, &value))
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

    bool Vec3Renderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.typeId != GetTypeID<Vec3>()) return false;

        f32 speed = 0.005f;
        Vec3&  vec3 = *static_cast<Vec3*>(value);
        if (ImGui::BeginTable("##vec3-table", 3))
        {
            DrawVecField("X", vec3.x, hasChanged, IM_COL32(138, 46, 61, 255), speed);
            DrawVecField("Y", vec3.y, hasChanged, IM_COL32(87, 121, 26, 255), speed);
            DrawVecField("Z", vec3.z, hasChanged, IM_COL32(43, 86, 138, 255), speed);
            ImGui::EndTable();
        }
        return true;
    }

    bool Vec2Renderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.typeId != GetTypeID<Vec2>()) return false;

        f32    speed = 0.005f;
        Vec2&  vec3 = *static_cast<Vec2*>(value);
        if (ImGui::BeginTable("##vec3-table", 2))
        {
            DrawVecField("X", vec3.x, hasChanged, IM_COL32(138, 46, 61, 255), speed);
            DrawVecField("Y", vec3.y, hasChanged, IM_COL32(87, 121, 26, 255), speed);
            ImGui::EndTable();
        }
        return true;
    }

    bool DrawColorPicker(Color& color)
    {
        if (ImGui::BeginPopup("color-picker"))
        {
            ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf |
                ImGuiColorEditFlags_AlphaBar;


            static f32 col[4] = {
                color.FloatRed(),
                color.FloatGreen(),
                color.FloatBlue(),
                color.FloatAlfa()
            };

            if (ImGui::ColorPicker4("##picker", col, flags))
            {
                Color::FromVec4(color, {col[0], col[1], col[2], col[3]});
            }
            ImGui::EndPopup();
            return true;
        }
        return false;
    }

    bool EnumRenderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (!typeInfo.isEnum) return false;
        TypeHandler* typeHandler = Registry::FindTypeById(typeInfo.typeId);
        if (!typeHandler) return true;

        u32 id = context->ReserveID();

        char str[25];
        sprintf(str, "###enumid%d", id);

        ImGui::SetNextItemWidth(-1);
        ValueHandler* valueHandler = typeHandler->FindValue(value);

        if (ImGui::BeginCombo(str, valueHandler != nullptr ? valueHandler->GetDesc().CStr() : ""))
        {
            for (const auto& valueHandler: typeHandler->GetValues())
            {
                if (ImGui::Selectable(valueHandler->GetDesc().CStr()))
                {
                    valueHandler->Update(value);
                    if (hasChanged)
                    {
                        *hasChanged = true;
                    }
                }
            }
            ImGui::EndCombo();
        }
        return true;
    }

    bool ColorRenderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.typeId != GetTypeID<Color>()) return false;

        Color& color = *static_cast<Color*>(value);
        const ImVec4 colV4(color.FloatRed(), color.FloatGreen(), color.FloatBlue(), color.FloatAlfa());

        u32 id = context->ReserveID();

        char str[25];
        sprintf(str, "###colorid%d", id);

        ImGui::SetNextItemWidth(-1);
        if (ImGui::ColorButton(str, colV4, 0, ImVec2(ImGui::CalcItemWidth(), 0)))
        {
            ImGui::OpenPopup("color-picker");
        }

        if (DrawColorPicker(color))
        {
            context->editingId = id;
        }
        else if (context->editingId == id)
        {
            context->editingId = U32_MAX;
            if (hasChanged)
            {
                *hasChanged = true;
            }
        }

        return true;
    }

    bool FloatRenderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.typeId != GetTypeID<f32>()) return false;

        u32 id = context->ReserveID();

        char str[25];
        sprintf(str, "###txtid%d", id);

        f32* floatValue = static_cast<f32*>(value);
        ImGui::SetNextItemWidth(-1);

        if (const UIFloatProperty* property = context->activeFieldHandler ? context->activeFieldHandler->GetAttribute<UIFloatProperty>() : nullptr)
        {
            if (ImGui::SliderFloat(str, floatValue, property->minValue, property->maxValue))
            {
                context->editingId = id;
            }
            else if (context->editingId == id && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                context->editingId = U32_MAX;
                if (hasChanged)
                {
                    *hasChanged = true;
                }
            }
            return true;
        }

        if (ImGui::InputFloat(str, floatValue))
        {
            if (hasChanged)
            {
                *hasChanged = true;
            }
        }
        return true;
    }

    bool BoolRenderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.typeId != GetTypeID<bool>()) return false;

        u32 id = context->ReserveID();
        char str[25];
        sprintf(str, "###txtid%d", id);

        bool* boolValue = static_cast<bool*>(value);

        if (ImGui::Checkbox(str, boolValue))
        {
            if (hasChanged)
            {
                *hasChanged = true;
            }
        }

        return true;
    }


    bool QuatRenderer(ImGui::DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged)
    {
        if (typeInfo.typeId != GetTypeID<Quat>()) return false;

        static int rotationMode = 0;

        f32    speed = 0.005f;
        Quat&  quat = *static_cast<Quat*>(value);

        if (rotationMode == 0)
        {
            Vec3 euler = Math::Degrees(Math::EulerAngles(quat));
            bool vecHasChanged = false;

            if (ImGui::BeginTable("##vec3-table", 3))
            {
                DrawVecField( "X", euler.x, &vecHasChanged, IM_COL32(138, 46, 61, 255), speed);
                DrawVecField( "Y", euler.y, &vecHasChanged, IM_COL32(87, 121, 26, 255), speed);
                DrawVecField( "Z", euler.z, &vecHasChanged, IM_COL32(43, 86, 138, 255), speed);
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
                DrawVecField("X", quat.x, hasChanged, IM_COL32(138, 46, 61, 255), speed);
                DrawVecField("Y", quat.y, hasChanged, IM_COL32(87, 121, 26, 255), speed);
                DrawVecField("Z", quat.z, hasChanged, IM_COL32(43, 86, 138, 255), speed);
                DrawVecField("W", quat.w, hasChanged, IM_COL32(84, 74, 119, 255), speed);
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

        return true;
    }

    void RegisterFieldRenderers()
    {
        AddFieldRenderer(ColorRenderer);
        AddFieldRenderer(FloatRenderer);
        AddFieldRenderer(BoolRenderer);
        AddFieldRenderer(Vec3Renderer);
        AddFieldRenderer(Vec2Renderer);
        AddFieldRenderer(QuatRenderer);
        AddFieldRenderer(DrawArrayField);
        AddFieldRenderer(EnumRenderer);
    }
}
