#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Action/AssetEditorActions.hpp"
#include "Fyrion/Editor/Action/SceneEditorAction.hpp"

namespace Fyrion
{
    SceneObjectAsset* SceneEditor::GetRootObject() const
    {
        return root;
    }

    void SceneEditor::ClearSelection()
    {
        selectedObjects.Clear();
        onSceneObjectAssetSelection.Invoke(nullptr);
    }

    void SceneEditor::SelectObject(SceneObjectAsset& object)
    {
        selectedObjects.Emplace(reinterpret_cast<usize>(&object));
        onSceneObjectAssetSelection.Invoke(&object);
    }

    void SceneEditor::DeselectObject(SceneObjectAsset& object)
    {
        selectedObjects.Erase(reinterpret_cast<usize>(&object));
    }

    bool SceneEditor::IsSelected(SceneObjectAsset& object) const
    {
        return selectedObjects.Has(reinterpret_cast<usize>(&object));
    }

    bool SceneEditor::IsParentOfSelected(SceneObjectAsset& object) const
    {
        for (const auto it : selectedObjects)
        {
            if (reinterpret_cast<SceneObjectAsset*>(it.first)->GetParent() == &object)
            {
                return true;
            }
        }
        return false;
    }

    void SceneEditor::DestroySelectedObjects()
    {
        if (root == nullptr) return;

        root->Modify();

        EditorTransaction* transaction = Editor::CreateTransaction();
        for (const auto it : selectedObjects)
        {
            transaction->CreateAction<DestroySceneObjectAction>(reinterpret_cast<SceneObjectAsset*>(it.first));
        }
        transaction->Commit();
        selectedObjects.Clear();
        onSceneObjectAssetSelection.Invoke(nullptr);
    }

    void SceneEditor::ClearSelectionStatic(VoidPtr userData)
    {
        static_cast<SceneEditor*>(userData)->ClearSelection();
    }

    void SceneEditor::CreateObject()
    {
        if (root == nullptr) return;

        root->Modify();

        if (selectedObjects.Empty())
        {
            EditorTransaction* transaction = Editor::CreateTransaction();
            transaction->AddPreExecute(this, ClearSelectionStatic);
            transaction->CreateAction<CreateSceneObjectAction>(*this, GetRootObject())->Commit();
        }
        else
        {
            EditorTransaction* transaction = Editor::CreateTransaction();
            transaction->AddPreExecute(this, ClearSelectionStatic);
            for (const auto it : selectedObjects)
            {
                transaction->CreateAction<CreateSceneObjectAction>(*this, reinterpret_cast<SceneObjectAsset*>(it.first));
            }
            onSceneObjectAssetSelection.Invoke(nullptr);
            selectedObjects.Clear();
            transaction->Commit();
        }
    }

    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    void SceneEditor::LoadScene(SceneObjectAsset* asset)
    {
        selectedObjects.Clear();
        root = asset;
    }

    bool SceneEditor::IsRootSelected() const
    {
        for (const auto it : selectedObjects)
        {
            if (reinterpret_cast<SceneObjectAsset*>(it.first) == root)
            {
                return true;
            }
        }
        return false;
    }

    void SceneEditor::RenameObject(SceneObjectAsset& asset, StringView newName)
    {
        Editor::CreateTransaction()->CreateAction<RenameAssetAction>(static_cast<Asset*>(&asset), newName)->Commit();
    }
}


