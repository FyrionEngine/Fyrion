#include "GraphEditorWindow.hpp"

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"

#include <imgui_canvas.h>
#include "builders.h"
#include "drawing.h"
#include "imgui_node_editor_internal.h"
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
        bool operator==(const ImRect& a, const ImRect& b)
        {
            return a.Min.x == b.Min.x && a.Min.y == b.Min.y && a.Max.x == b.Max.x && a.Max.y == b.Max.y;
        }


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

    GraphEditorWindow::GraphEditorWindow() : m_graphEditor(Editor::GetAssetTree())
    {
    }

    void GraphEditorWindow::Init(u32 id, VoidPtr userData)
    {
        if (userData == nullptr) return;

        m_graphEditor.OpenGraph(*static_cast<RID*>(userData));

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
        ImVec2 mousePos = ed::ScreenToCanvas(ImGui::GetMousePos());
        m_mousePos = {mousePos.x, mousePos.y};
    }

    void GraphEditorWindow::Draw(u32 id, bool& open)
    {
        if (!m_graphEditor.IsGraphLoaded()) return;

        static ImRect lastRect = {};

        ed::Detail::EditorContext* editor = static_cast<ed::Detail::EditorContext*>(m_editorContext);

        ax::NodeEditor::Utilities::BlueprintNodeBuilder builder{nullptr, 64, 64};

        auto&           style = ImGui::GetStyle();
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::SetNextWindowSize({800 * style.ScaleFactor, 400 * style.ScaleFactor}, ImGuiCond_Once);
        ImGui::Begin(id, ICON_FA_DIAGRAM_PROJECT " Graph Editor", &open);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            lastRect = editor->GetViewRect();
        }

        SetCurrentEditor();

        ed::Begin("Node Graph Editor", ImVec2(0.0, 0.0f));

        Span<GraphEditorNodePin*> pins = m_graphEditor.GetPins();


        for (GraphEditorNode* node : m_graphEditor.GetNodes())
        {
            if (!node->initialized)
            {
                ed::SetNodePosition(node->rid.id, ImVec2(node->position.x, node->position.y));
                node->initialized = true;
            }

            builder.Begin(node->rid.id);
            builder.Header(GetNodeColor(node->label));

            ImGui::Spring(0);

            ImGui::Text("%s", node->label.CStr());

            ImGui::Spring(1);
            const float nodeHeaderHeight = 18.0f;
            ImGui::Dummy(ImVec2(0, nodeHeaderHeight));
            ImGui::Spring(0);
            builder.EndHeader();


            for (u32 pinId : node->inputs)
            {
                const GraphEditorNodePin& input = *pins[pinId];
                builder.Input(input.pin);
                DrawPinIcon(input.typeId, false);
                ImGui::Spring(0);
                ImGui::TextUnformatted(input.label.CStr());
                ImGui::Spring(0);
                builder.EndInput();
            }

            for (u32 pinId : node->outputs)
            {
                const GraphEditorNodePin& output = *pins[pinId];
                builder.Output(output.pin);
                ImGui::TextUnformatted(output.label.CStr());
                ImGui::Spring(0);
                DrawPinIcon(output.typeId, false);
                builder.EndOutput();
            }
            builder.End();
        }

        for (GraphEditorLink& link : m_graphEditor.GetLinks())
        {
            ed::Link(link.linkId, link.inputPin, link.outputPin, GetTypeColor(link.linkType), 3.f);
        }

        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                if (inputPinId && outputPinId && inputPinId != outputPinId)
                {
                    if (m_graphEditor.ValidateLink(inputPinId.Get(), static_cast<u64>(outputPinId)))
                    {
                        if (ed::AcceptNewItem())
                        {
                            m_graphEditor.AddLink(inputPinId.Get(), static_cast<u64>(outputPinId));
                        }
                    }
                    else
                    {
                        ed::RejectNewItem();
                    }
                }
            }
        }

        ed::EndCreate();

        if (ed::BeginDelete())
        {

            ed::NodeId deletedNodeId{};
            while(ed::QueryDeletedNode(&deletedNodeId))
            {

            }

            ed::LinkId deletedLinkId{};
            while (ed::QueryDeletedLink(&deletedLinkId))
            {

            }

            ed::EndDelete();
        }

        ed::End();

        ed::SetCurrentEditor(nullptr);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && editor->GetViewRect() == lastRect)
        {
            m_clickMousePos = m_mousePos;
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

    void GraphEditorWindow::AddOutputAction(const MenuItemEventData& eventData)
    {
        GraphEditorWindow* graphEditorWindow = static_cast<GraphEditorWindow*>(eventData.drawData);
        graphEditorWindow->m_graphEditor.AddOutput(static_cast<TypeHandler*>(eventData.itemData), graphEditorWindow->m_clickMousePos);
    }

    void GraphEditorWindow::AddNodeAction(const MenuItemEventData& eventData)
    {
        GraphEditorWindow* graphEditorWindow = static_cast<GraphEditorWindow*>(eventData.drawData);
        graphEditorWindow->m_graphEditor.AddNode(static_cast<FunctionHandler*>(eventData.itemData), graphEditorWindow->m_clickMousePos);
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
        Event::Bind<OnInit, &OnInitGraphEditor>();

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
