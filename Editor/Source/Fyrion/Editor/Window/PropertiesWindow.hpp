#pragma once

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    struct GraphEditorNodePin;
    struct MenuItemEventData;
    class SceneEditor;
    struct SceneObject;
    class SceneObjectAsset;
    class GraphEditor;

    class PropertiesWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);

        PropertiesWindow();
        ~PropertiesWindow() override;
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<PropertiesWindow>& type);
    private:
        u32                 m_idCount{};
        SceneEditor&        m_sceneEditor;
        String              m_stringCache{};
        SceneObjectAsset*   selectedObject{};
        bool                m_renamingFocus{};
        String              m_renamingCache{};
        SceneObjectAsset*   m_renamingObject{};
        GraphEditorNodePin* m_renmaingPin{};
        String              m_searchComponentString{};
        //RID                 m_selectedComponent = {};
        GraphEditorNodePin* m_selectedNodePin = {};

        static void OpenProperties(const MenuItemEventData& eventData);
        void        DrawSceneObject(u32 id, SceneObjectAsset& object);
//      void        DrawGraphNode(GraphEditor* graphEditor, RID node);

        void SceneObjectAssetSelection(SceneObjectAsset* objectAsset);

        u32 PushId()
        {
            m_idCount = m_idCount + 5;
            return m_idCount;
        }
    };
}