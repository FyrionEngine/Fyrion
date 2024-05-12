#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    struct SceneObject;
    class SceneEditor;

    class SceneTreeWindow : public EditorWindow
    {
    public:
        SceneTreeWindow();
        void Draw(u32 id, bool& open) override;

        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<SceneTreeWindow>& type);

    private:
        SceneEditor& m_sceneEditor;
        String       m_searchEntity{};
        String       m_nameCache{};
        bool         m_renamingSelected{};
        bool         m_entityIsSelected{};

        void        DrawSceneObject(RID object);
        static void OpenSceneTree(const MenuItemEventData& eventData);
        static void AddSceneObject(const MenuItemEventData& eventData);
        static void AddSceneObjectFromAsset(const MenuItemEventData& eventData);
        static void AddComponent(const MenuItemEventData& eventData);
        static void RenameSceneObject(const MenuItemEventData& eventData);
        static void DuplicateSceneObject(const MenuItemEventData& eventData);
        static void DeleteSceneObject(const MenuItemEventData& eventData);

        static MenuItemContext s_menuItemContext;
    };
}
