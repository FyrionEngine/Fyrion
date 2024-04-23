#include "PropertiesWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Scene/SceneEditor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Scene/SceneAssets.hpp"

namespace Fyrion
{
    void PropertiesWindow::Draw(u32 id, bool& open)
    {
        ImGui::Begin(id, ICON_FA_CIRCLE_INFO " Properties", &open, ImGuiWindowFlags_NoScrollbar);

        if (RID object = Editor::GetSceneEditor().GetLastSelectedObject())
        {
            DrawSceneObject(id, object);
        }

        ImGui::End();
    }

    void PropertiesWindow::OpenProperties(VoidPtr userData)
    {
        Editor::OpenWindow<PropertiesWindow>();
    }

    void PropertiesWindow::DrawSceneObject(u32 id, RID rid)
    {
        SceneEditor& sceneEditor = Editor::GetSceneEditor();

        bool root = sceneEditor.GetRootObject() == rid;
        ResourceObject read = Repository::Read(rid);

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

            ImGuiInputTextFlags nameFlags = 0;
            if (readOnly || root)
            {
                nameFlags |= ImGuiInputTextFlags_ReadOnly;
            }

            StringView objectName = root ? sceneEditor.GetRootName() : read[SceneObjectAsset::Name].Value<StringView>();
            m_StringCache = objectName;
            u32 hash = HashValue(rid);

            if (ImGui::InputText(hash, m_StringCache, nameFlags))
            {
                m_renamingCache = objectName;
                m_renamingFocus = true;
                m_renamingObject = rid;
            }

            if (!ImGui::IsItemActive() && m_renamingFocus)
            {
                Editor::GetSceneEditor().RenameObject(m_renamingObject, m_renamingCache);
                m_renamingObject = {};
                m_renamingFocus = false;
                m_renamingCache.Clear();
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("UUID");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            String uuid = ToString(Repository::GetUUID(rid));
            ImGui::InputText(hash + 10, uuid, ImGuiInputTextFlags_ReadOnly);
            ImGui::EndDisabled();
            ImGui::EndTable();

        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        f32 width = ImGui::GetContentRegionAvail().x;
        auto size = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;

        ImGui::BeginHorizontal("horizontal-01", ImVec2(width, size));

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

        RID prototype = Repository::GetPrototype(rid);
        if (prototype)
        {
            ImGui::BeginHorizontal(9999, ImVec2(width, size));
            ImGui::Spring(1.f);

            if (ImGui::BorderedButton("horizontal-01-02", ImVec2((width * 2) / 3, size)))
            {
                int a = 0;
                //openAsset = entityData.rid;
            }

            ImGui::Spring(1.f);
            ImGui::EndHorizontal();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        // draw components


        Array<RID> components = read.GetSubObjectSetAsArray(SceneObjectAsset::Components);
        for (RID component : components)
        {

        }

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
                            Editor::GetSceneEditor().AddComponent(rid, typeHandler);
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
