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


    SceneObjectAction::SceneObjectAction(SceneEditor& sceneEditor, SceneObjectActionType type) : sceneEditor(sceneEditor), type(type)
    {
        // current = AssetDatabase::Create<SceneObjectAsset>();
        // current->SetName("New Object");
        // current->SetUUID(UUID::RandomUUID());
        // pos = parent->GetChildren().Count();
    }

    void SceneObjectAction::Commit()
    {
        sceneEditor.GetScene()->Modify();

        switch (type)
        {
            case SceneObjectActionType::Create:
            {
                sceneEditor.SelectObject(*current);
                break;
            }
            case SceneObjectActionType::Destroy:
            {
                break;
            }
            case SceneObjectActionType::Rename:
            {
                break;
            }
        }

        // parent->GetChildren().Insert(current, pos);
        // current->SetActive(true);
    }

    void SceneObjectAction::Rollback()
    {
        sceneEditor.GetScene()->Modify();

        // sceneEditor.DeselectObject(*current);
        // current->SetActive(false);
        // parent->GetChildren().Remove(current);
    }

    void SceneObjectAction::RegisterType(NativeTypeHandler<SceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObjectActionType>();
    }

    void InitSceneEditorAction()
    {
        Registry::Type<OpenSceneAction>();
        Registry::Type<SceneObjectAction>();
    }
}
