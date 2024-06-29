#include "SceneEditorAction.hpp"

#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"

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


    CreateSceneObjectAction::CreateSceneObjectAction(SceneEditor& sceneEditor, SceneObject* parent) : sceneEditor(sceneEditor), parent(parent), current(nullptr)
    {
        // current = AssetDatabase::Create<SceneObjectAsset>();
        // current->SetName("New Object");
        // current->SetUUID(UUID::RandomUUID());
        // pos = parent->GetChildren().Count();
    }

    CreateSceneObjectAction::~CreateSceneObjectAction()
    {
        // if (current != nullptr && !current->IsActive())
        // {
        //     AssetDatabase::Destroy(current);
        // }
    }

    void CreateSceneObjectAction::Commit()
    {
        // sceneEditor.SelectObject(*current);
        // parent->GetChildren().Insert(current, pos);
        // current->SetActive(true);
    }

    void CreateSceneObjectAction::Rollback()
    {
        // sceneEditor.DeselectObject(*current);
        // current->SetActive(false);
        // parent->GetChildren().Remove(current);
    }

    void CreateSceneObjectAction::RegisterType(NativeTypeHandler<CreateSceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*>();
    }

    DestroySceneObjectAction::DestroySceneObjectAction(SceneObject* object) : object(object), parent(object->GetParent())
    {
    }

    DestroySceneObjectAction::~DestroySceneObjectAction()
    {
        // if (!object->IsActive())
        // {
        //     AssetDatabase::Destroy(object);
        // }
    }

    void DestroySceneObjectAction::Commit()
    {
        // object->SetActive(false);
        // pos = parent->GetChildren().IndexOf(object);
        // if (pos != nPos)
        // {
        //     parent->GetChildren().Remove(object);\
        // }
    }

    void DestroySceneObjectAction::Rollback()
    {
        // if (pos != nPos)
        // {
        //     parent->GetChildren().Insert(object, pos);
        // }
        // object->SetActive(true);
    }

    void DestroySceneObjectAction::RegisterType(NativeTypeHandler<DestroySceneObjectAction>& type)
    {
        type.Constructor<SceneObject*>();
    }

    void InitSceneEditorAction()
    {
        Registry::Type<OpenSceneAction>();
        Registry::Type<CreateSceneObjectAction>();
        Registry::Type<DestroySceneObjectAction>();
    }
}
