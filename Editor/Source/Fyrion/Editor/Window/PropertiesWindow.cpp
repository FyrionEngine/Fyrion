#include "PropertiesWindow.hpp"

#include "GraphEditorWindow.hpp"
#include "imgui_internal.h"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Action/AssetEditorActions.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    PropertiesWindow::PropertiesWindow() : m_sceneEditor(Editor::GetSceneEditor())
    {
        Event::Bind<OnSceneObjectSelection, &PropertiesWindow::SceneObjectSelection>(this);
        Event::Bind<OnAssetSelection, &PropertiesWindow::AssetSelection>(this);
    }

    PropertiesWindow::~PropertiesWindow()
    {
        Event::Unbind<OnSceneObjectSelection, &PropertiesWindow::SceneObjectSelection>(this);
        Event::Unbind<OnAssetSelection, &PropertiesWindow::AssetSelection>(this);
    }

    void PropertiesWindow::Draw(u32 id, bool& open)
    {
        m_idCount = id;

        ImGui::Begin(id, ICON_FA_CIRCLE_INFO " Properties", &open, ImGuiWindowFlags_NoScrollbar);

        if (selectedObject)
        {
            DrawSceneObject(id, *selectedObject);
        }
        else if (selectedAsset)
        {
            DrawAsset(selectedAsset);
        }
        ImGui::End();
    }

    void PropertiesWindow::ClearSelection()
    {
        selectedAsset = nullptr;
        selectedObject = nullptr;
        selectedComponent = nullptr;
    }

    void PropertiesWindow::OpenProperties(const MenuItemEventData& eventData)
    {
        Editor::OpenWindow<PropertiesWindow>();
    }

    void PropertiesWindow::DrawSceneObject(u32 id, SceneObject& object)
    {
        bool root = m_sceneEditor.GetRootObject() == &object;

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

            m_stringCache = object.GetName();
            u32 hash = HashValue(reinterpret_cast<usize>(&object));

            if (ImGui::InputText(hash, m_stringCache, nameFlags))
            {
                m_renamingCache = m_stringCache;
                m_renamingFocus = true;
                m_renamingObject = &object;
            }

            if (!ImGui::IsItemActive() && m_renamingFocus)
            {
                m_sceneEditor.RenameObject(*m_renamingObject, m_renamingCache);
                m_renamingObject = {};
                m_renamingFocus = false;
                m_renamingCache.Clear();
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("UUID");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);

            String uuid = ToString(object.GetUUID());
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

        if (object.GetPrototype() != nullptr)
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

        for (Component* component : object.GetComponents())
        {
            bool propClicked = false;
            bool open = ImGui::CollapsingHeaderProps(HashValue(reinterpret_cast<usize>(component)), FormatName(component->typeHandler->GetSimpleName()).CStr(), &propClicked);
            if (propClicked)
            {
                openComponentSettings = true;
                selectedComponent = component;
            }

            if (open)
            {
                ImGui::BeginDisabled(object.GetPrototype() != nullptr && !object.IsComponentOverride(component));
                ImGui::Indent();
                ImGui::DrawType(ImGui::DrawTypeDesc{
                    .itemId = reinterpret_cast<usize>(component),
                    .typeHandler = component->typeHandler,
                    .instance = component,
                    .flags = readOnly ? ImGuiDrawTypeFlags_ReadOnly : 0u,
                    .userData = this,
                    .callback = [](ImGui::DrawTypeDesc& desc, VoidPtr newValue)
                    {
                        PropertiesWindow* propertiesWindow = static_cast<PropertiesWindow*>(desc.userData);

                        propertiesWindow->m_sceneEditor.UpdateComponent(propertiesWindow->selectedObject,
                                                                        static_cast<Component*>(desc.instance),
                                                                        static_cast<Component*>(newValue));
                    },
                });
                ImGui::Unindent();
                ImGui::EndDisabled();
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
                        String name = FormatName(typeHandler->GetSimpleName());
                        if (ImGui::Selectable(name.CStr()))
                        {
                            m_sceneEditor.AddComponent(object, typeHandler);
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
                m_sceneEditor.ResetComponent(object, selectedComponent);
                ImGui::CloseCurrentPopup();
            }

            if (object.GetPrototype() != nullptr && !object.IsComponentOverride(selectedComponent))
            {
                if (ImGui::MenuItem("Override"))
                {
                    m_sceneEditor.OverridePrototypeComponent(&object, selectedComponent);
                }
            }

            if (object.GetPrototype() != nullptr && object.IsComponentOverride(selectedComponent))
            {
                if (ImGui::MenuItem("Remove prototype override"))
                {
                    m_sceneEditor.RemoveOverridePrototypeComponent(&object, selectedComponent);
                }
            }

            if (ImGui::MenuItem("Remove"))
            {
                m_sceneEditor.RemoveComponent(object, selectedComponent);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopupMenu(popupOpenSettings);
    }

    void PropertiesWindow::DrawAsset(AssetHandler* assetHandler)
    {
        bool readOnly = false;

        Asset* asset = AssetManager::LoadById(assetHandler->GetUUID());

        auto& style = ImGui::GetStyle();
        if (ImGui::BeginTable("#asset-table", 2))
        {
            String name = assetHandler->GetName();
            String path = assetHandler->GetPath();
            UUID   uuid = assetHandler->GetUUID();

            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText(HashValue(name), name, ImGuiInputTextFlags_ReadOnly);


            if (!path.Empty())
            {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Path");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText(HashValue(path), path, ImGuiInputTextFlags_ReadOnly);
            }

            if (uuid)
            {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("UUID");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1);
                String uuidStr = ToString(uuid);
                ImGui::InputText(HashValue(uuid.firstValue), uuidStr, ImGuiInputTextFlags_ReadOnly);
            }
            ImGui::EndTable();
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

        ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
        if (ImGui::CollapsingHeader(assetHandler->GetDisplayName().CStr(), 0))
        {
            ImGui::Indent();
            ImGui::DrawType(ImGui::DrawTypeDesc{
                .itemId = reinterpret_cast<usize>(asset),
                .typeHandler = assetHandler->GetType(),
                .instance = asset,
                .flags = readOnly ? ImGuiDrawTypeFlags_ReadOnly : 0u,
                .userData = this,
                .callback = [](ImGui::DrawTypeDesc& desc, VoidPtr newValue)
                {
                    Editor::CreateTransaction()->CreateAction<UpdateAssetAction>(static_cast<Asset*>(desc.instance), static_cast<Asset*>(newValue))->Commit();
                },
            });
            ImGui::Unindent();

            if (AssetManager::CanReimportAsset(assetHandler))
            {
                f32  width = ImGui::GetContentRegionAvail().x;
                auto size = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * style.ScaleFactor);

                ImGui::BeginHorizontal("horizontal-02", ImVec2(width, size));

                ImGui::Spring(1.f);
                if (ImGui::BorderedButton("Reimport", ImVec2(width * 2 / 3, size)))
                {
                    AssetManager::ReimportAsset(assetHandler);
                }
                ImGui::Spring(1.f);

                ImGui::EndHorizontal();
            }
        }
    }

    void PropertiesWindow::SceneObjectSelection(SceneObject* objectAsset)
    {
        if (objectAsset == nullptr && selectedObject == nullptr) return;
        ClearSelection();
        selectedObject = objectAsset;
    }

    void PropertiesWindow::AssetSelection(AssetHandler* assetHandler)
    {
        if (selectedAsset == nullptr && assetHandler == nullptr) return;

        // if (assetHandler == nullptr || assetHandler->LoadInstance() != nullptr)
        // {
        //     ClearSelection();
        //     selectedAsset = assetHandler;
        // }
    }

#if FY_ASSET_REFACTOR
    void PropertiesWindow::DrawGraphNode(GraphEditor* graphEditor, RID node)
    {
        bool readOnly = false;

        ImGuiInputTextFlags nameFlags = 0;
        if (readOnly)
        {
            nameFlags |= ImGuiInputTextFlags_ReadOnly;
        }

        GraphEditorNode* graphNode = graphEditor->GetNodeByRID(node);

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

            m_stringCache = graphNode->label;

            if (ImGui::InputText(PushId(), m_stringCache, nameFlags))
            {
                m_renamingCache = m_stringCache;
                m_renamingFocus = true;
                m_renamingObject = node;
            }

            if (!ImGui::IsItemActive() && m_renamingFocus && m_renamingObject == node)
            {
                graphEditor->RenameNode(graphNode, m_renamingCache);
                m_renamingObject = {};
                m_renamingFocus = false;
                m_renamingCache.Clear();
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("UUID");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            String uuid = ToString(Repository::GetUUID(node));
            ImGui::InputText(PushId(), uuid, ImGuiInputTextFlags_ReadOnly);

            ImGui::EndDisabled();
            ImGui::EndTable();
        }

        for (GraphEditorNodePin* pin : graphNode->inputs)
        {

        }

        bool openPopup = false;

        for (GraphEditorNodePin* pin : graphNode->outputs)
        {
            ImGui::BeginDisabled(!pin->editable);
            TypeHandler* typeHandler = Registry::FindTypeById(pin->typeId);
            if (typeHandler)
            {
                bool propClicked = false;
                bool open = ImGui::CollapsingHeaderProps(reinterpret_cast<usize>(pin), pin->label.CStr(), &propClicked);
                if (propClicked)
                {
                    openPopup = true;
                    m_selectedNodePin = pin;
                }

                if (open)
                {
                    if (ImGui::BeginTable("##node-table", 2))
                    {
                        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.4f);
                        ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);

                        ImGui::BeginDisabled(readOnly);

                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();

                        ImGui::Text("Name");
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(-1);

                        m_stringCache = pin->label;

                        if (ImGui::InputText(PushId(), m_stringCache, nameFlags))
                        {
                            m_renamingCache = m_stringCache;
                            m_renamingFocus = true;
                            m_renmaingPin = pin;
                        }

                        if (!ImGui::IsItemActive() && m_renamingFocus && m_renmaingPin == pin)
                        {
                            graphEditor->RenamePin(pin, m_renamingCache);
                            m_renmaingPin = {};
                            m_renamingFocus = false;
                            m_renamingCache.Clear();
                        }

                        ImGui::TableNextColumn();
                        ImGui::Text("Public Value");
                        ImGui::TableNextColumn();

                        bool publicValue = pin->publicValue;

                        if (ImGui::Checkbox("###aaaaa", &publicValue))
                        {
                            graphEditor->ChangePinVisibility(pin, publicValue);
                        }

                        ImGui::EndDisabled();
                        ImGui::EndTable();
                    }
                }
            }
            ImGui::EndDisabled();
        }

        if (openPopup)
        {
            ImGui::OpenPopup("open-node-settings");
        }

        bool popupOpenSettings = ImGui::BeginPopupMenu("open-node-settings", 0, false);
        if (popupOpenSettings)
        {
            if (ImGui::MenuItem("Remove"))
            {
                graphEditor->DeletePin(m_selectedNodePin);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopupMenu(popupOpenSettings);
    }
#endif

    void PropertiesWindow::RegisterType(NativeTypeHandler<PropertiesWindow>& type)
    {
        Editor::AddMenuItem(MenuItemCreation{.itemName = "Window/Properties", .action = OpenProperties});

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::BottomRight,
            .createOnInit = true
        });
    }

    void InitPropertiesWindow()
    {
        Registry::Type<PropertiesWindow>();
    }
}
