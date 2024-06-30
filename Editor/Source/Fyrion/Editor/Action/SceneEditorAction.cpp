#include "SceneEditorAction.hpp"

#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"
#include "Fyrion/Scene/SceneManager.hpp"

namespace Fyrion
{
    OpenSceneAction::OpenSceneAction(SceneEditor& sceneEditor, SceneObjectAsset* scene) : sceneEditor(sceneEditor)
    {
        oldScene = sceneEditor.GetScene();
        newScene = scene;
    }

    void OpenSceneAction::Commit()
    {
        sceneEditor.LoadScene(newScene);
    }

    void OpenSceneAction::Rollback()
    {
        sceneEditor.LoadScene(oldScene);
    }

    void OpenSceneAction::RegisterType(NativeTypeHandler<OpenSceneAction>& type)
    {
        type.Constructor<SceneEditor, SceneObjectAsset*>();
    }


    CreateSceneObjectAction::CreateSceneObjectAction(SceneEditor& sceneEditor, SceneObject* parent) : sceneEditor(sceneEditor), parent(parent)
    {
        current = SceneManager::CreateObject();
        current->SetName("New Object");
        current->SetUUID(UUID::RandomUUID());
        pos = parent->GetChildren().Size();
    }

    CreateSceneObjectAction::~CreateSceneObjectAction()
    {
        if (current != nullptr)
        {
            SceneManager::Destroy(current);
        }
    }

    void CreateSceneObjectAction::Commit()
    {
        sceneEditor.SelectObject(*current);
        parent->AddChildAt(current, pos);
        sceneEditor.Modify();
    }

    void CreateSceneObjectAction::Rollback()
    {
        sceneEditor.DeselectObject(*current);
        parent->RemoveChild(current);
        sceneEditor.Modify();
    }

    void CreateSceneObjectAction::RegisterType(NativeTypeHandler<CreateSceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*>();
    }

    DestroySceneObjectAction::DestroySceneObjectAction(SceneEditor& sceneEditor, SceneObject* object) : sceneEditor(sceneEditor), object(object), parent(object->GetParent()) {}

    DestroySceneObjectAction::~DestroySceneObjectAction()
    {
        if (object)
        {
            SceneManager::Destroy(object);
        }
    }

    void DestroySceneObjectAction::RegisterType(NativeTypeHandler<DestroySceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*>();
    }

    void DestroySceneObjectAction::Commit()
    {
        pos = parent->GetChildren().IndexOf(object);
        if (pos != nPos)
        {
            parent->GetChildren().Remove(pos);
            sceneEditor.Modify();
        }
    }

    void DestroySceneObjectAction::Rollback()
    {
        if (pos != nPos)
        {
            parent->AddChildAt(object, pos);
            sceneEditor.Modify();
        }
    }

    RenameSceneObjectAction::RenameSceneObjectAction(SceneEditor& sceneEditor, SceneObject* sceneObject, StringView newName)
        : sceneEditor(sceneEditor),
          object(sceneObject),
          newName(newName),
          oldName(sceneObject->GetName()) {}

    void RenameSceneObjectAction::RegisterType(NativeTypeHandler<RenameSceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, StringView>();
    }

    void RenameSceneObjectAction::Commit()
    {
        object->SetName(newName);
        sceneEditor.Modify();
    }

    void RenameSceneObjectAction::Rollback()
    {
        object->SetName(oldName);
        sceneEditor.Modify();
    }

    void InitSceneEditorAction()
    {
        Registry::Type<OpenSceneAction>();
        Registry::Type<CreateSceneObjectAction>();
        Registry::Type<DestroySceneObjectAction>();
        Registry::Type<RenameSceneObjectAction>();
    }
}
