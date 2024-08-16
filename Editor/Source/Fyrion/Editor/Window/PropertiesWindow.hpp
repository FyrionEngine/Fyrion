#pragma once

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    class Component;
    struct GraphEditorNodePin;
    struct MenuItemEventData;
    class SceneEditor;
    class SceneObject;
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
        SceneObject*        selectedObject{};
        AssetHandler*          selectedAsset{};
        bool                m_renamingFocus{};
        String              m_renamingCache{};
        SceneObject*        m_renamingObject{};
        GraphEditorNodePin* m_renmaingPin{};
        String              m_searchComponentString{};
        Component*          selectedComponent = {};
        GraphEditorNodePin* m_selectedNodePin = {};

        void ClearSelection();

        static void OpenProperties(const MenuItemEventData& eventData);
        void        DrawSceneObject(u32 id, SceneObject& object);
        void        DrawAsset(AssetHandler* assetHandler);
        //      void        DrawGraphNode(GraphEditor* graphEditor, RID node);

        void SceneObjectSelection(SceneObject* objectAsset);
        void AssetSelection(AssetHandler* asset);

        u32 PushId()
        {
            m_idCount = m_idCount + 5;
            return m_idCount;
        }
    };
}
