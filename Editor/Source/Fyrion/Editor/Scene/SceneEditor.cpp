#include "SceneEditor.hpp"

#include "Fyrion/Editor/Asset/AssetEditor.hpp"

namespace Fyrion
{
    Scene* SceneEditor::GetScene() const
    {
        return scene;
    }

    AssetFile* SceneEditor::GetAssetFile() const
    {
        return assetFile;
    }

    void SceneEditor::SetScene(AssetFile* assetFile)
    {
        this->assetFile = assetFile;
        scene = Assets::Load<Scene>(assetFile->uuid);
    }

    void SceneEditor::ClearSelection()
    {
        selectedObjects.Clear();
        onGameObjectSelectionHandler.Invoke(nullptr);
    }

    void SceneEditor::SelectObject(GameObject& object)
    {
        selectedObjects.Emplace(&object);
        onGameObjectSelectionHandler.Invoke(&object);
    }

    void SceneEditor::DeselectObject(GameObject& object)
    {
        selectedObjects.Erase(&object);
    }

    bool SceneEditor::IsSelected(GameObject& object) const
    {
        return selectedObjects.Has(&object);
    }

    bool SceneEditor::IsParentOfSelected(GameObject& object) const
    {
        return false;
    }

    void SceneEditor::RenameObject(GameObject& object, StringView newName)
    {
        object.SetName(newName);
        assetFile->currentVersion++;
    }

    void SceneEditor::DestroySelectedObjects()
    {
        for (auto it : selectedObjects)
        {
            it.first->Destroy();
        }
        ClearSelection();
    }

    void SceneEditor::CreateGameObject()
    {
        GameObject* gameObject = scene->GetRootObject().CreateChild();
        gameObject->SetName("GameObject");

        assetFile->currentVersion++;
    }

    bool SceneEditor::IsValidSelection()
    {
        return scene != nullptr;
    }

    void SceneEditor::AddComponent(GameObject* gameObject, TypeHandler* typeHandler)
    {

    }

    void SceneEditor::ResetComponent(GameObject* gameObject, Component* component)
    {

    }

    void SceneEditor::RemoveComponent(GameObject* gameObject, Component* component)
    {

    }

    void SceneEditor::DoUpdate()
    {
        if (scene)
        {
            scene->FlushQueues();
        }
    }
}
