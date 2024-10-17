#pragma once
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"


namespace Fyrion {
    class GameObject;
}

namespace Fyrion
{
    class SceneEditor;

    class SceneTreeWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);

        SceneTreeWindow();

        void Draw(u32 id, bool& open) override;
        void DrawGameObject(GameObject& gameObject);

        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<SceneTreeWindow>& type);
        static void OpenSceneTree(const MenuItemEventData& eventData);
        static void AddSceneObject(const MenuItemEventData& eventData);
        static void AddSceneObjectFromAsset(const MenuItemEventData& eventData);
        static void AddComponent(const MenuItemEventData& eventData);
        static void RenameSceneObject(const MenuItemEventData& eventData);
        static void DuplicateSceneObject(const MenuItemEventData& eventData);
        static void DeleteSceneObject(const MenuItemEventData& eventData);
        static bool CheckSelectedObject(const MenuItemEventData& eventData);

    private:
        SceneEditor& sceneEditor;
        String       searchObject{};
        String       stringCache{};
        bool         renamingFocus{};
        bool         renamingSelected{};
        String       renamingStringCache{};

        void        CheckDragDropAsset();

        static MenuItemContext menuItemContext;
    };
}
