#pragma once
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Scene/Scene.hpp"


namespace Fyrion
{
    class SceneEditor
    {
    public:
        Scene* GetActiveScene() const;
        void   SetActiveScene(Scene* scene);
        void   ClearSelection();
        void   SelectObject(GameObject& object);
        void   DeselectObject(GameObject& object);
        bool   IsSelected(GameObject& object) const;
        bool   IsParentOfSelected(GameObject& object) const;
        void   RenameObject(GameObject& object, StringView newName);
        void   DestroySelectedObjects();
        void   CreateGameObject();
        bool   IsValidSelection();

    private:
        Scene*               activeScene = nullptr;
        HashSet<GameObject*> selectedObjects{};

        EventHandler<OnGameObjectSelection> onGameObjectSelectionHandler{};
    };
}
