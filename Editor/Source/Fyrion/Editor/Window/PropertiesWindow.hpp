#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    class SceneEditor;
    struct SceneObject;

    class PropertiesWindow : public EditorWindow
    {
    public:
        PropertiesWindow();
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<PropertiesWindow>& type);
    private:
        SceneEditor&        m_sceneEditor;
        String              m_stringCache{};
        bool				m_renamingFocus{};
        String				m_renamingCache{};
        RID                 m_renamingObject{};
        String              m_searchComponentString{};
        RID                 m_selectedComponent = {};

        static void OpenProperties(VoidPtr userData);
        void        DrawSceneObject(u32 id, RID rid);
    };
}
