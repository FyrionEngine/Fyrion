#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"

namespace Fyrion
{
    class SceneTreeWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;

        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<SceneTreeWindow>& type);
    private:
        static void OpenSceneTree(VoidPtr userData);

        static void     AddGameObject(VoidPtr userData);
        static void     AddGameObjectFromAsset(VoidPtr userData);
        static void     AddComponent(VoidPtr userData);
        static void     RenameGameObject(VoidPtr userData);
        static void     DuplicateGameObject(VoidPtr userData);
        static void     DeleteGameObject(VoidPtr userData);

        static MenuItemContext s_menuItemContext;
    };
}
