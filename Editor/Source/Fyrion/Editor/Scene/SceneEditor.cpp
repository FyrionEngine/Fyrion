#include "SceneEditor.hpp"

namespace Fyrion
{
    Scene* SceneEditor::GetActiveScene() const
    {
        return activeScene;
    }

    void SceneEditor::SetActiveScene(Scene* scene)
    {
        activeScene = scene;
    }

    void SceneEditor::ClearSelection()
    {

    }
    void SceneEditor::SelectObject(GameObject& object)
    {

    }
    void SceneEditor::DeselectObject(GameObject& object)
    {

    }
    bool SceneEditor::IsSelected(GameObject& object) const
    {
        return false;
    }
    bool SceneEditor::IsParentOfSelected(GameObject& object) const
    {
        return false;
    }
    void SceneEditor::RenameObject(GameObject& object, StringView newName) {}
    void SceneEditor::DestroySelectedObjects() {}
    void SceneEditor::CreateGameObject()
    {

    }

    bool SceneEditor::IsValidSelection()
    {
        return true;
    }
}
