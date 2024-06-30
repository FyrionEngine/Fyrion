#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Action/AssetEditorActions.hpp"
#include "Fyrion/Editor/Action/SceneEditorAction.hpp"

namespace Fyrion
{
    SceneObject* SceneEditor::GetRootObject() const
    {
        if (scene)
        {
            return &scene->GetObject();
        }
        return nullptr;
    }

    void SceneEditor::ClearSelection()
    {
        selectedObjects.Clear();
        onSceneObjectAssetSelection.Invoke(nullptr);
    }

    void SceneEditor::SelectObject(SceneObject& object)
    {
        selectedObjects.Emplace(reinterpret_cast<usize>(&object));
        onSceneObjectAssetSelection.Invoke(&object);
    }

    void SceneEditor::DeselectObject(SceneObject& object)
    {
        selectedObjects.Erase(reinterpret_cast<usize>(&object));
    }

    bool SceneEditor::IsSelected(SceneObject& object) const
    {
        return selectedObjects.Has(reinterpret_cast<usize>(&object));
    }

    bool SceneEditor::IsParentOfSelected(SceneObject& object) const
    {
        for (const auto it : selectedObjects)
        {
            if (reinterpret_cast<SceneObject*>(it.first)->GetParent() == &object)
            {
                return true;
            }
        }
        return false;
    }

    void SceneEditor::DestroySelectedObjects()
    {
        if (scene == nullptr) return;

        scene->Modify();

        EditorTransaction* transaction = Editor::CreateTransaction();
        for (const auto it : selectedObjects)
        {
            SceneObjectAction* action = transaction->CreateAction<SceneObjectAction>(*this, SceneObjectActionType::Create);
            action->current = reinterpret_cast<SceneObject*>(it.first);
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
        if (scene == nullptr) return;

        scene->Modify();

        if (selectedObjects.Empty())
        {
            EditorTransaction* transaction = Editor::CreateTransaction();
            transaction->AddPreExecute(this, ClearSelectionStatic);
            SceneObjectAction* action = transaction->CreateAction<SceneObjectAction>(*this, SceneObjectActionType::Create);
            action->parent = GetRootObject();
            action->Commit();
        }
        else
        {
            EditorTransaction* transaction = Editor::CreateTransaction();
            transaction->AddPreExecute(this, ClearSelectionStatic);
            for (const auto it : selectedObjects)
            {
                SceneObjectAction* action = transaction->CreateAction<SceneObjectAction>(*this, SceneObjectActionType::Create);
                action->parent = reinterpret_cast<SceneObject*>(it.first);
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
        scene = asset;
    }

    SceneObjectAsset* SceneEditor::GetScene() const
    {
        return scene;
    }

    bool SceneEditor::IsRootSelected() const
    {
        for (const auto it : selectedObjects)
        {
            if (reinterpret_cast<SceneObject*>(it.first) == &scene->GetObject())
            {
                return true;
            }
        }
        return false;
    }

    void SceneEditor::AddComponent(SceneObject& asset, TypeHandler* typeHandler)
    {

    }

    void SceneEditor::RenameObject(SceneObject& asset, StringView newName)
    {
        SceneObjectAction* action = Editor::CreateTransaction()->CreateAction<SceneObjectAction>(*this, SceneObjectActionType::Rename);
        action->current = &asset;
        action->newName = newName;
        action->Commit();
    }
}


