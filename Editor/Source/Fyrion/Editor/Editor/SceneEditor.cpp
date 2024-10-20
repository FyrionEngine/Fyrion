#include "SceneEditor.hpp"

#include <Fyrion/Scene/SceneTypes.hpp>

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Action/AssetEditorActions.hpp"
#include "Fyrion/Editor/Action/SceneEditorAction.hpp"

namespace Fyrion
{
    SceneObject* SceneEditor::GetRootObject() const
    {
        if (scene)
        {
            return scene->GetObject();
        }
        return nullptr;
    }

    void SceneEditor::Modify() const
    {
        if (scene)
        {
            scene->SetModified();
        }
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
        for (const auto& it : selectedObjects)
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

        EditorTransaction* transaction = Editor::CreateTransaction();
        for (const auto it : selectedObjects)
        {
            transaction->CreateAction<DestroySceneObjectAction>(*this, reinterpret_cast<SceneObject*>(it.first));
        }
        transaction->Commit();
        selectedObjects.Clear();
        onSceneObjectAssetSelection.Invoke(nullptr);

        scene->SetModified();
    }

    void SceneEditor::ClearSelectionStatic(VoidPtr userData)
    {
        static_cast<SceneEditor*>(userData)->ClearSelection();
    }

    const HashSet<usize>& SceneEditor::GetSelectedObjects() const
    {
        return selectedObjects;
    }

    void SceneEditor::CreateObject(SceneObjectAsset* prototype)
    {
        if (scene == nullptr) return;

        if (selectedObjects.Empty())
        {
            EditorTransaction* transaction = Editor::CreateTransaction();
            transaction->AddPreExecute(this, ClearSelectionStatic);
            transaction->CreateAction<CreateSceneObjectAction>(*this, GetRootObject(), prototype)->Commit();
        }
        else
        {
            EditorTransaction* transaction = Editor::CreateTransaction();
            transaction->AddPreExecute(this, ClearSelectionStatic);
            for (const auto it : selectedObjects)
            {
                transaction->CreateAction<CreateSceneObjectAction>(*this, reinterpret_cast<SceneObject*>(it.first), prototype);
            }
            onSceneObjectAssetSelection.Invoke(nullptr);
            selectedObjects.Clear();
            transaction->Commit();
        }

        scene->SetModified();
    }

    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    void SceneEditor::StartSimulation()
    {

    }

    void SceneEditor::StopSimulation()
    {

    }

    void SceneEditor::LoadScene(SceneObjectAsset* asset)
    {
        ClearSelection();

        if (scene)
        {
            GetRootObject()->SetActive(false);
        }

        scene = asset;

        if (scene)
        {
            GetRootObject()->SetActive(true);
        }
    }

    SceneObjectAsset* SceneEditor::GetScene() const
    {
        return scene;
    }

    bool SceneEditor::IsRootSelected() const
    {
        for (const auto it : selectedObjects)
        {
            if (reinterpret_cast<SceneObject*>(it.first) == scene->GetObject())
            {
                return true;
            }
        }
        return false;
    }

    void SceneEditor::AddComponent(SceneObject& object, TypeHandler* typeHandler)
    {
        Editor::CreateTransaction()->CreateAction<AddComponentSceneObjectAction>(*this, &object, typeHandler)->Commit();
    }

    void SceneEditor::ResetComponent(SceneObject& object, Component* component)
    {
        Component* newValue = component->typeHandler->Cast<Component>(component->typeHandler->NewInstance());
        Editor::CreateTransaction()->CreateAction<UpdateComponentSceneObjectAction>(*this, component, newValue);
        component->typeHandler->Destroy(newValue);
    }

    void SceneEditor::RemoveComponent(SceneObject& object, Component* component)
    {
        Editor::CreateTransaction()->CreateAction<RemoveComponentObjectAction>(*this, &object, component)->Commit();
    }

    void SceneEditor::UpdateComponent(SceneObject* sceneObject, Component* component, Component* newValue)
    {
        Editor::CreateTransaction()->CreateAction<UpdateComponentSceneObjectAction>(*this, component, newValue);
    }

    void SceneEditor::OverridePrototypeComponent(SceneObject* object, Component* component)
    {
        Editor::CreateTransaction()->CreateAction<OverridePrototypeComponentAction>(*this, object, component)->Commit();
    }
    void SceneEditor::RemoveOverridePrototypeComponent(SceneObject* object, Component* component)
    {
        Editor::CreateTransaction()->CreateAction<RemoveOverridePrototypeComponentAction>(*this, object, component)->Commit();
    }

    void SceneEditor::RenameObject(SceneObject& asset, StringView newName)
    {
        Editor::CreateTransaction()->CreateAction<RenameSceneObjectAction>(*this, &asset, newName)->Commit();
    }
}
