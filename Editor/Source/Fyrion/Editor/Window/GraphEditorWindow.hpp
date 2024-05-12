#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    enum class GraphEditorType
    {
        None          = 0,
        ResourceGraph = 1,
        BehaviorGraph = 2
    };

    class FY_API GraphEditorWindow : public EditorWindow
    {
    public:
        void Init(u32 id, VoidPtr userData) override;
        void Draw(u32 id, bool& open) override;

        static void OpenGraphWindow(RID graphId);
        static void RegisterType(NativeTypeHandler<GraphEditorWindow>& type);


        void AddOutput(TypeHandler* outputType);
        void AddNode(FunctionHandler* functionHandler);
    private:
        RID     m_assetId{};
        RID     m_graph{};
        TypeID  m_graphTypeId{};
        String  m_graphName{};
        VoidPtr m_editorContext{};
        String  m_searchNodeString{};

        Array<RID> m_nodesCache{};

        void SetCurrentEditor();

        static MenuItemContext s_menuItemContext;
        static void OnInitGraphEditor();

        static void AddOutputAction(const MenuItemEventData& eventData);
        static void AddNodeAction(const MenuItemEventData& eventData);
    };
}
