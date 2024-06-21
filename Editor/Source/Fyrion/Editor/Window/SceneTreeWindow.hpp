#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"

namespace Fyrion
{
    struct SceneObject;
    struct SceneEditor;

    class SceneTreeWindow : public EditorWindow
    {
    public:

        FY_BASE_TYPES(EditorWindow);

        SceneTreeWindow();
        void Draw(u32 id, bool& open) override;

        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<SceneTreeWindow>& type);

    private:
        SceneEditor& sceneEditor;
        String       searchEntity{};
        String       nameCache{};
        bool         renamingSelected{};
        bool         entityIsSelected{};

        void        DrawSceneObject(SceneObject& sceneObject);
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
