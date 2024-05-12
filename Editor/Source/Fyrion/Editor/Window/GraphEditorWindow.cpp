#include "GraphEditorWindow.hpp"

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

#include <imgui_canvas.h>
#include "builders.h"
#include "drawing.h"
#include "widgets.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Assets/AssetTypes.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/GraphTypes.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"


namespace ed = ax::NodeEditor;

namespace Fyrion
{

    MenuItemContext GraphEditorWindow::s_menuItemContext = {};

    namespace
    {
        const int pinIconSize = 24;

        inline ImColor GetNodeColor(const StringView& nodeName)
        {
            ImColor color = ImColor(ImHashStr(nodeName.CStr(), nodeName.Size(), 8876832));
            color = ImColor(
                Math::Clamp(color.Value.x, 0.3f, 0.7f),
                Math::Clamp(color.Value.y, 0.3f, 0.7f),
                Math::Clamp(color.Value.z, 0.3f, 0.7f),
                1.0f);

            return color;
        }

        inline ImColor GetTypeColor(TypeID typeId)
        {
            ImColor color = ImColor(ImHashData(&typeId, sizeof(typeId), 8876832));
            color = ImColor(
                Math::Clamp(color.Value.x, 0.3f, 0.7f),
                Math::Clamp(color.Value.y, 0.3f, 0.7f),
                Math::Clamp(color.Value.z, 0.3f, 0.7f),
                1.0f);
            return color;
        }

        void DrawPinIcon(TypeID typeId, bool connected)
        {
            ax::Drawing::IconType iconType = ax::Drawing::IconType::Circle;
            ax::Widgets::Icon(ImVec2(static_cast<float>(pinIconSize), static_cast<float>(pinIconSize)), iconType, connected, GetTypeColor(typeId), ImColor(32, 32, 32, 255));
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
            m_nodesCache.Resize(object.GetSubObjectSetCount(ResourceGraphAsset::Nodes));
            object.GetSubObjectSet(ResourceGraphAsset::Nodes, m_nodesCache);
        }

        ed::Begin("Node Graph Editor", ImVec2(0.0, 0.0f));

        for(const RID& node: m_nodesCache)
        {
            builder.Begin(ed::NodeId(node.id));

            ResourceObject nodeObject = Repository::Read(node);
            TypeHandler* typeHandler = nullptr;
            FunctionHandler* functionHandler = nullptr;

            String label = nodeObject[GraphNodeAsset::Label].Value<String>();

            if (nodeObject.Has(GraphNodeAsset::NodeFunction))
            {
                functionHandler = Registry::FindFunctionByName(nodeObject[GraphNodeAsset::NodeFunction].As<String>());
                builder.Header(GetNodeColor(functionHandler->GetName()));
            }
            else if (nodeObject.Has(GraphNodeAsset::NodeOutput))
            {
                typeHandler = Registry::FindTypeByName(nodeObject[GraphNodeAsset::NodeOutput].As<String>());
                builder.Header(GetNodeColor(typeHandler->GetName()));
            }

            ImGui::Spring(0);


            if (!label.Empty())
            {
                ImGui::Text("%s", label.CStr());
            }
            else if (typeHandler)
            {
                ImGui::Text("%s", FormatName(typeHandler->GetSimpleName()).CStr());
            }
            else if (functionHandler)
            {
                ImGui::Text("%s", FormatName(functionHandler->GetSimpleName()).CStr());
            }

            ImGui::Spring(1);
            const float nodeHeaderHeight = 18.0f;
            ImGui::Dummy(ImVec2(0, nodeHeaderHeight));
            ImGui::Spring(0);
            builder.EndHeader();


            if (typeHandler)
            {
                Span<FieldHandler*> fields = typeHandler->GetFields();
                for (FieldHandler* field : fields)
                {
                    if (field->HasAttribute<GraphInput>())
                    {
                        builder.Input(HashValue(field->GetName()));
                        DrawPinIcon(field->GetFieldInfo().typeInfo.typeId, false);
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(FormatName(field->GetName()).CStr());
                        ImGui::Spring(0);
                        builder.EndInput();
                    }
                }
            }
            else if (functionHandler)
            {

                Span<ParamHandler> params = functionHandler->GetParams();
                for(const ParamHandler& param : params)
                {
                    if (param.HasAttribute<GraphInput>())
                    {
                        builder.Input(HashValue(StringView{param.GetName()}));
                        DrawPinIcon(param.GetFieldInfo().typeInfo.typeId, false);
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(FormatName(param.GetName()).CStr());
                        ImGui::Spring(0);
                        builder.EndInput();
                    }
                }

                for(const ParamHandler& param : params)
                {
                    if (param.HasAttribute<GraphOutput>())
                    {
                        builder.Output(HashValue(StringView{param.GetName()}));
                        ImGui::TextUnformatted(FormatName(param.GetName()).CStr());
                        ImGui::Spring(0);
                        DrawPinIcon(param.GetFieldInfo().typeInfo.typeId, false);
                        builder.EndOutput();
                    }
                }
            }


            builder.End();
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

    void GraphEditorWindow::AddOutput(TypeHandler* outputType)
    {
        RID nodeAsset = Repository::CreateResource<GraphNodeAsset>();
        ResourceObject nodeObject = Repository::Write(nodeAsset);
        nodeObject[GraphNodeAsset::NodeOutput] = outputType->GetName();
        nodeObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(ResourceGraphAsset::Nodes, nodeAsset);
        graphAsset.Commit();
    }

    void GraphEditorWindow::AddNode(FunctionHandler* functionHandler)
    {
        RID nodeAsset = Repository::CreateResource<GraphNodeAsset>();
        ResourceObject nodeObject = Repository::Write(nodeAsset);
        nodeObject[GraphNodeAsset::NodeFunction] = functionHandler->GetName();
        nodeObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(ResourceGraphAsset::Nodes, nodeAsset);
        graphAsset.Commit();
    }

    void GraphEditorWindow::AddOutputAction(const MenuItemEventData& eventData)
    {
        GraphEditorWindow* graphEditorWindow = static_cast<GraphEditorWindow*>(eventData.drawData);
        graphEditorWindow->AddOutput(static_cast<TypeHandler*>(eventData.itemData));
    }

    void GraphEditorWindow::AddNodeAction(const MenuItemEventData& eventData)
    {
        GraphEditorWindow* graphEditorWindow = static_cast<GraphEditorWindow*>(eventData.drawData);
        graphEditorWindow->AddNode(static_cast<FunctionHandler*>(eventData.itemData));
    }

    void GraphEditorWindow::OnInitGraphEditor()
    {
        Span<TypeHandler*> outputs = Registry::FindTypesByAttribute<ResourceGraphOutput>();
        for (TypeHandler* outputType : outputs)
        {
            const ResourceGraphOutput* resourceGraphOutput = outputType->GetAttribute<ResourceGraphOutput>();

            s_menuItemContext.AddMenuItem(MenuItemCreation{
                .itemName = !resourceGraphOutput->label.Empty() ? resourceGraphOutput->label : FormatName(outputType->GetSimpleName()),
                .action = AddOutputAction,
                .menuData = outputType
            });
        }

        Span<FunctionHandler*> functions = Registry::FindFunctionsByAttribute<ResourceGraphNode>();

        for (FunctionHandler* function : functions)
        {
            const ResourceGraphNode* resourceGraphNode = function->GetAttribute<ResourceGraphNode>();

            s_menuItemContext.AddMenuItem(MenuItemCreation{
                .itemName = !resourceGraphNode->label.Empty() ? resourceGraphNode->label : FormatName(function->GetSimpleName()),
                .action = AddNodeAction,
                .menuData = function
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
