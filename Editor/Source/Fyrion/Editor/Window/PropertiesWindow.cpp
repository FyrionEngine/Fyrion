#include "PropertiesWindow.hpp"

#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/ImGui/ImGuiEditor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    PropertiesWindow::PropertiesWindow() : sceneEditor(Editor::GetSceneEditor())
    {
        Event::Bind<OnGameObjectSelection, &PropertiesWindow::GameObjectSelection>(this);
    }

    PropertiesWindow::~PropertiesWindow() {}

    void PropertiesWindow::DrawSceneObject(u32 id, GameObject* gameObject)
    {
        bool root = &sceneEditor.GetScene()->GetRootObject() == gameObject;

        ImGuiStyle& style = ImGui::GetStyle();
        bool        readOnly = false;

        ImGuiInputTextFlags nameFlags = 0;
        if (readOnly || root)
        {
            nameFlags |= ImGuiInputTextFlags_ReadOnly;
        }

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

            stringCache = root ? StringView{sceneEditor.GetAssetFile()->fileName} : gameObject->GetName();

            u32 hash = HashValue(reinterpret_cast<usize>(&gameObject));

            if (ImGui::InputText(hash, stringCache, nameFlags))
            {
                renamingCache = stringCache;
                renamingFocus = true;
                renamingObject = gameObject;
            }

            if (!ImGui::IsItemActive() && renamingFocus)
            {
                sceneEditor.RenameObject(*renamingObject, renamingCache);
                renamingObject = {};
                renamingFocus = false;
                renamingCache.Clear();
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("UUID");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);

            String uuid = gameObject->GetUUID().ToString();
            ImGui::InputText(hash + 10, uuid, ImGuiInputTextFlags_ReadOnly);
            ImGui::EndDisabled();
            ImGui::EndTable();
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        f32  width = ImGui::GetContentRegionAvail().x;
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

        if (gameObject->GetPrototype() != nullptr)
        {
            ImGui::BeginHorizontal(9999, ImVec2(width, size));
            ImGui::Spring(1.f);

            if (ImGui::BorderedButton("Open Prototype", ImVec2((width * 2) / 3, size)))
            {
                //TODO defer open scene.
            }

            ImGui::Spring(1.f);
            ImGui::EndHorizontal();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        bool openComponentSettings = false;

        for (Component* component : gameObject->GetComponents())
        {
            TypeHandler* typeHandler = Registry::FindTypeById(component->typeId);

            bool propClicked = false;
            bool open = ImGui::CollapsingHeaderProps(HashValue(reinterpret_cast<usize>(component)), FormatName(typeHandler->GetSimpleName()).CStr(), &propClicked);
            if (propClicked)
            {
                openComponentSettings = true;
                selectedComponent = component;
            }

            if (open)
            {
                // ImGui::BeginDisabled(object.GetPrototype() != nullptr && !object.IsComponentOverride(component));
                ImGui::Indent();
                ImGui::DrawType(ImGui::DrawTypeDesc{
                    .itemId = reinterpret_cast<usize>(component),
                    .typeHandler = typeHandler,
                    .instance = component,
                    .flags = readOnly ? ImGui::ImGuiDrawTypeFlags_ReadOnly : 0u,
                    .userData = this,
                    .callback = [](ImGui::DrawTypeDesc& desc, VoidPtr newValue)
                    {
                        PropertiesWindow* propertiesWindow = static_cast<PropertiesWindow*>(desc.userData);

                        propertiesWindow->sceneEditor.UpdateComponent(propertiesWindow->selectedObject,
                                                                      static_cast<Component*>(desc.instance),
                                                                      static_cast<Component*>(newValue));
                    },
                });
                ImGui::Unindent();
                // ImGui::EndDisabled();
            }
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
            ImGui::SearchInputText(id + 100, searchComponentString);
            ImGui::Separator();

            TypeHandler* componentHandler = Registry::FindType<Component>();
            if (componentHandler)
            {
                for (DerivedType& derivedType : componentHandler->GetDerivedTypes())
                {
                    TypeHandler* typeHandler = Registry::FindTypeById(derivedType.typeId);
                    if (typeHandler)
                    {
                        String name = FormatName(typeHandler->GetSimpleName());
                        if (ImGui::Selectable(name.CStr()))
                        {
                            sceneEditor.AddComponent(gameObject, typeHandler);
                        }
                    }
                }
            }
        }
        ImGui::EndPopupMenu(popupRes);

        if (openComponentSettings)
        {
            ImGui::OpenPopup("open-component-settings");
        }

        bool popupOpenSettings = ImGui::BeginPopupMenu("open-component-settings", 0, false);
        if (popupOpenSettings)
        {
            if (ImGui::MenuItem("Reset"))
            {
                sceneEditor.ResetComponent(gameObject, selectedComponent);
                ImGui::CloseCurrentPopup();
            }

            // if (gameObject->GetPrototype() != nullptr && !gameObject.IsComponentOverride(selectedComponent))
            // {
            //     if (ImGui::MenuItem("Override"))
            //     {
            //         sceneEditor.OverridePrototypeComponent(&gameObject, selectedComponent);
            //     }
            // }
            //
            // if (gameObject->GetPrototype() != nullptr && gameObject.IsComponentOverride(selectedComponent))
            // {
            //     if (ImGui::MenuItem("Remove prototype override"))
            //     {
            //         sceneEditor.RemoveOverridePrototypeComponent(&gameObject, selectedComponent);
            //     }
            // }

            if (ImGui::MenuItem("Remove"))
            {
                sceneEditor.RemoveComponent(gameObject, selectedComponent);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopupMenu(popupOpenSettings);
    }

    void PropertiesWindow::Draw(u32 id, bool& open)
    {
        ImGui::Begin(id, ICON_FA_CIRCLE_INFO " Properties", &open, ImGuiWindowFlags_NoScrollbar);
        if (selectedObject)
        {
            DrawSceneObject(id, selectedObject);
        }
        ImGui::End();
    }


    void PropertiesWindow::ClearSelection()
    {
        selectedObject = nullptr;
        selectedComponent = nullptr;
    }

    void PropertiesWindow::OpenProperties(const MenuItemEventData& eventData)
    {
        Editor::OpenWindow<PropertiesWindow>();
    }

    void PropertiesWindow::GameObjectSelection(GameObject* object)
    {
        if (object == nullptr && selectedObject == nullptr) return;
        ClearSelection();
        selectedObject = object;
    }


    void PropertiesWindow::RegisterType(NativeTypeHandler<PropertiesWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Properties", .action = OpenProperties});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::BottomRight,
            .createOnInit = true
        });
    }
}
