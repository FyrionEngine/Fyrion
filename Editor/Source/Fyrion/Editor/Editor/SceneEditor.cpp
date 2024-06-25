#include "SceneEditor.hpp"

namespace Fyrion
{
    SceneObjectAsset* SceneEditor::GetRootObject() const
    {
        return root;
    }

    void SceneEditor::ClearSelection() {}
    void SceneEditor::SelectObject(SceneObjectAsset& object) {}
    bool SceneEditor::IsSelected(SceneObjectAsset& object) const
    {
        return false;
    }
    bool SceneEditor::IsParentOfSelected(SceneObjectAsset& object) const
    {
        return false;
    }
    void SceneEditor::DestroySelectedObjects() {}
    void SceneEditor::CreateObject() {}
    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    void SceneEditor::LoadScene(SceneObjectAsset* asset)
    {
        root = asset;
    }
}
