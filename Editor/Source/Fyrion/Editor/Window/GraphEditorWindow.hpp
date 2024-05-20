#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Editor/Editor/GraphEditor.hpp"
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
        GraphEditorWindow();
        void Init(u32 id, VoidPtr userData) override;
        void Draw(u32 id, bool& open) override;

        static void OpenGraphWindow(RID graphId);
        static void RegisterType(NativeTypeHandler<GraphEditorWindow>& type);

    private:
        GraphEditor m_graphEditor;

        VoidPtr m_editorContext{};
        String  m_searchNodeString{};
        Vec2    m_mousePos{};
        Vec2    m_clickMousePos{};

        Vec2                m_comboOpenPos{};
        bool                m_openPopup{};
        TypeHandler*        m_comboOpenType{};
        GraphEditorNodePin* m_comboPin{};

        void SetCurrentEditor();

        static MenuItemContext s_menuItemContext;
        static void OnInitGraphEditor();

        static void AddOutputAction(const MenuItemEventData& eventData);
        static void AddNodeAction(const MenuItemEventData& eventData);

        void DrawPinInputField(GraphEditorNodePin* pin);
    };
}
