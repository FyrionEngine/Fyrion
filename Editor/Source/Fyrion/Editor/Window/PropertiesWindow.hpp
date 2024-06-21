#pragma once

#if FY_ASSET_REFACTOR

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    struct GraphEditorNodePin;
    struct MenuItemEventData;
    class SceneEditor;
    struct SceneObject;
    class GraphEditor;

    class PropertiesWindow : public EditorWindow
    {
    public:
        PropertiesWindow();
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<PropertiesWindow>& type);
    private:
        u32                 m_idCount{};
        SceneEditor&        m_sceneEditor;
        String              m_stringCache{};
        bool                m_renamingFocus{};
        String              m_renamingCache{};
        RID                 m_renamingObject{};
        GraphEditorNodePin* m_renmaingPin{};
        String              m_searchComponentString{};
        RID                 m_selectedComponent = {};
        GraphEditorNodePin* m_selectedNodePin = {};

        static void OpenProperties(const MenuItemEventData& eventData);
        void        DrawSceneObject(u32 id, RID rid);
        void        DrawGraphNode(GraphEditor* graphEditor, RID node);


        u32 PushId()
        {
            m_idCount = m_idCount + 5;
            return m_idCount;
        }
    };
}

#endif