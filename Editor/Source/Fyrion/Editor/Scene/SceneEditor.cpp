#include "SceneEditor.hpp"

#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Scene/Component/Component.hpp"

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
        for (const auto& it : selectedObjects)
        {
            GameObject* parent = it.first->GetParent();
            if (parent)
            {
                if (parent == &object)
                {
                    return true;
                }

                if (IsParentOfSelected(*parent))
                {
                    return true;
                }
            }
        }
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
        if (scene == nullptr) return;


        if (selectedObjects.Empty())
        {
            GameObject* gameObject = scene->GetRootObject().CreateChild();
            gameObject->SetName("Object");
            SelectObject(*gameObject);
        } else
        {
            Array<GameObject*> newObjects;
            newObjects.Reserve(selectedObjects.Size());

            for(auto it: selectedObjects)
            {
                GameObject* gameObject = it.first->CreateChild();
                gameObject->SetName("Object");
                newObjects.EmplaceBack(gameObject);
            }

            ClearSelection();

            for(GameObject* object : newObjects)
            {
                SelectObject(*object);
            }
        }
        assetFile->currentVersion++;
    }

    bool SceneEditor::IsValidSelection()
    {
        return scene != nullptr;
    }

    void SceneEditor::AddComponent(GameObject* gameObject, TypeHandler* typeHandler)
    {
        gameObject->AddComponent(typeHandler->GetTypeInfo().typeId);
    }

    void SceneEditor::ResetComponent(GameObject* gameObject, Component* component)
    {

    }

    void SceneEditor::RemoveComponent(GameObject* gameObject, Component* component)
    {
        gameObject->RemoveComponent(component);
    }

    void SceneEditor::UpdateComponent(GameObject* gameObject, Component* instance, Component* newValue)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(newValue->typeId))
        {
            typeHandler->Copy(newValue, instance);
            instance->OnChange();
        }
    }

    void SceneEditor::DoUpdate()
    {
        if (scene)
        {
            scene->FlushQueues();
        }
    }
}
