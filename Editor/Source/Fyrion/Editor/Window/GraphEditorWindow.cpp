#include "GraphEditorWindow.hpp"

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

#include <imgui_canvas.h>
#include "builders.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Assets/AssetTypes.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"


namespace ed = ax::NodeEditor;

namespace Fyrion
{

    MenuItemContext GraphEditorWindow::s_menuItemContext = {};

    namespace
    {
        ImColor GetNodeColor(const String& nodeName)
        {
            ImColor color = ImColor(ImHashStr(nodeName.CStr(), nodeName.Size(), 8876832));
            return ImColor(color.Value.x, color.Value.y, color.Value.z, 1.0f);
        }
    }

    void GraphEditorWindow::Init(u32 id, VoidPtr userData)
    {
        if (userData == nullptr) return;

        m_assetId = *static_cast<RID*>(userData);

        ResourceObject asset = Repository::Read(m_assetId);
        m_graph = asset[Asset::Object].Value<RID>();
        m_graphName = asset[Asset::Name].Value<String>();
        m_graphTypeId = Repository::GetResourceTypeId(Repository::GetResourceType(m_graph));

        ed::Config config{};
        config.SettingsFile = nullptr;

        m_editorContext = ed::CreateEditor(&config);

        SetCurrentEditor();

        auto& editorStyle = ed::GetStyle();
        editorStyle.NodeRounding = 2.0f * ImGui::GetStyle().ScaleFactor;
        editorStyle.NodeBorderWidth = 2.0f;
        editorStyle.HoveredNodeBorderWidth = 2.0f * ImGui::GetStyle().ScaleFactor;
        editorStyle.SelectedNodeBorderWidth = 2.0f * ImGui::GetStyle().ScaleFactor;

        editorStyle.Colors[ed::StyleColor_Bg] = ImColor(20, 21, 23, 255);
        editorStyle.Colors[ed::StyleColor_Grid] = ImColor(38, 39, 31, 255);
        editorStyle.Colors[ed::StyleColor_NodeBg] = ImColor(28, 31, 33, 255);
        editorStyle.Colors[ed::StyleColor_NodeBorder] = ImColor(51, 54, 62, 140);

        ed::SetCurrentEditor(nullptr);
    }

    void GraphEditorWindow::SetCurrentEditor()
    {
        ed::SetCurrentEditor((ed::EditorContext*)m_editorContext);
    }

    void GraphEditorWindow::Draw(u32 id, bool& open)
    {
        if (!m_graph) return;

        ax::NodeEditor::Utilities::BlueprintNodeBuilder builder{nullptr, 64, 64};

        auto& style = ImGui::GetStyle();
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::SetNextWindowSize({800 * style.ScaleFactor, 400 * style.ScaleFactor}, ImGuiCond_Once);
        ImGui::Begin(id, ICON_FA_DIAGRAM_PROJECT " Graph Editor", &open);

        SetCurrentEditor();

        if (m_graphTypeId == GetTypeID<ResourceGraphAsset>())
        {
            ResourceObject object = Repository::Read(m_graph);
            m_nodesCache.Resize(object.GetSubObjectSetCount(ResourceGraphAsset::Links));
            object.GetSubObjectSet(ResourceGraphAsset::Links, m_nodesCache);
        }

        ed::Begin("Node Graph Editor", ImVec2(0.0, 0.0f));

        for(const RID& node: m_nodesCache)
        {
            builder.Begin(ed::NodeId(node.id));

            ResourceObject nodeObject = Repository::Read(node);

            //header
            {
                if (nodeObject.Has(GraphNodeAsset::NodeFunction))
                {
                    builder.Header(GetNodeColor(nodeObject[GraphNodeAsset::NodeFunction].As<String>()));
                }
                else if (nodeObject.Has(GraphNodeAsset::NodeOutput))
                {
                    builder.Header(GetNodeColor(nodeObject[GraphNodeAsset::NodeOutput].As<String>()));
                }

                ImGui::Spring(1);
                const float nodeHeaderHeight = 18.0f;
                ImGui::Dummy(ImVec2(0, nodeHeaderHeight));
                ImGui::Spring(0);
                builder.EndHeader();

                // builder.Header(GetNodeColor(node->name));
                // ImGui::Spring(0);
                //
                // ImGui::Text("%s", node->label.CStr());
                //
            }
        }


        ed::End();
        ed::SetCurrentEditor(nullptr);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            ImGui::OpenPopup("add-node-popup");
        }

        auto popupRes = ImGui::BeginPopupMenu("add-node-popup", 0, false);
        if (popupRes)
        {
            ImGui::SearchInputText(id + 100, m_searchNodeString);
            ImGui::Separator();
            s_menuItemContext.Draw(this);
        }

        ImGui::EndPopupMenu(popupRes);
        ImGui::End();
    }

    void GraphEditorWindow::OpenGraphWindow(RID graphId)
    {
        Editor::OpenWindow(GetTypeID<GraphEditorWindow>(), &graphId);
    }

    void GraphEditorWindow::AddNewNodeOutputAction(const MenuItemEventData& eventData)
    {
        GraphEditorWindow* graphEditorWindow = static_cast<GraphEditorWindow*>(eventData.drawData);
        TypeHandler* outputType = static_cast<TypeHandler*>(eventData.itemData);



    }

    void GraphEditorWindow::OnInitGraphEditor()
    {
        Span<TypeHandler*> outputs = Registry::FindTypesByAttribute<ResourceGraphOutput>();
        for (TypeHandler* outputType : outputs)
        {
            const ResourceGraphOutput* resourceGraphOutput = outputType->GetAttribute<ResourceGraphOutput>();

            s_menuItemContext.AddMenuItem(MenuItemCreation{
                .itemName = !resourceGraphOutput->label.Empty() ? resourceGraphOutput->label : FormatName(outputType->GetSimpleName()),
                .action = AddNewNodeOutputAction,
                .menuData = outputType
            });
        }
    }

    void GraphEditorWindow::RegisterType(NativeTypeHandler<GraphEditorWindow>& type)
    {
        Event::Bind<OnInit , &OnInitGraphEditor>();

        type.Attribute<EditorWindowProperties>(EditorWindowProperties{
            .dockPosition = DockPosition::Center,
            .createOnInit = false
        });
    }

    void InitGraphEditorWindow()
    {
        Registry::Type<GraphEditorWindow, EditorWindow>();
    }
}
