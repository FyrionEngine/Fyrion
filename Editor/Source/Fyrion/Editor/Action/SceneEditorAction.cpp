#include "SceneEditorAction.hpp"

#include "imgui_internal.h"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Asset/AssetSerialization.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
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
        if (object && !object->IsAlive())
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
            parent->RemoveChildAt(pos);
        }

        sceneEditor.Modify();
        parent->SetAlive(false);
    }

    void DestroySceneObjectAction::Rollback()
    {
        if (pos != nPos)
        {
            parent->AddChildAt(object, pos);

            sceneEditor.Modify();
        }
        parent->SetAlive(true);
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

    AddComponentSceneObjectAction::AddComponentSceneObjectAction(SceneEditor& sceneEditor, SceneObject* object, TypeHandler* typeHandler)
        : sceneEditor(sceneEditor),
          object(object),
          typeHandler(typeHandler),
          component(nullptr) {}

    void AddComponentSceneObjectAction::Commit()
    {
        sceneEditor.Modify();
        component = &object->AddComponent(typeHandler);
    }

    void AddComponentSceneObjectAction::Rollback()
    {
        sceneEditor.Modify();
        object->RemoveComponent(component);
    }

    void AddComponentSceneObjectAction::RegisterType(NativeTypeHandler<AddComponentSceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, TypeHandler*>();
    }

    UpdateComponentSceneObjectAction::UpdateComponentSceneObjectAction(SceneEditor& sceneEditor, Component* component, Component* newValue) : sceneEditor(sceneEditor), component(component)
    {
        JsonAssetWriter writer;
        currentStrValue = JsonAssetWriter::Stringify(component->typeHandler->Serialize(writer, component));
        newStrValue = JsonAssetWriter::Stringify(component->typeHandler->Serialize(writer, newValue));

        JsonAssetReader reader(newStrValue);
        component->typeHandler->Deserialize(reader, reader.ReadObject(), component);

        sceneEditor.Modify();
        component->OnChange();
        ImGui::ClearDrawType(reinterpret_cast<usize>(component));
    }

    void UpdateComponentSceneObjectAction::Commit()
    {
        JsonAssetReader reader(newStrValue);
        component->typeHandler->Deserialize(reader, reader.ReadObject(), component);

        ImGui::ClearDrawType(reinterpret_cast<usize>(component));
        ImGui::ClearTextData();

        sceneEditor.Modify();
        component->OnChange();
    }

    void UpdateComponentSceneObjectAction::Rollback()
    {
        JsonAssetReader reader(currentStrValue);
        component->typeHandler->Deserialize(reader, reader.ReadObject(), component);

        ImGui::ClearDrawType(reinterpret_cast<usize>(component));
        ImGui::ClearTextData();

        sceneEditor.Modify();
        component->OnChange();
    }

    void UpdateComponentSceneObjectAction::RegisterType(NativeTypeHandler<UpdateComponentSceneObjectAction>& type)
    {
        type.Constructor<SceneEditor, Component*, Component*>();
    }

    RemoveComponentObjectAction::RemoveComponentObjectAction(SceneEditor& sceneEditor, SceneObject* object, Component* component) : sceneEditor(sceneEditor),
                                                                                                                                    component(component),
                                                                                                                                    typeHandler(component->typeHandler),
                                                                                                                                    object(object) {}

    void RemoveComponentObjectAction::Commit()
    {
        JsonAssetWriter writer;
        value = JsonAssetWriter::Stringify(typeHandler->Serialize(writer, component));
        object->RemoveComponent(component);
        sceneEditor.Modify();
        typeHandler->Destroy(component);
    }

    void RemoveComponentObjectAction::Rollback()
    {
        component = &object->AddComponent(typeHandler);

        JsonAssetReader reader(value);
        typeHandler->Deserialize(reader, reader.ReadObject(), component);

        sceneEditor.Modify();
    }

    void RemoveComponentObjectAction::RegisterType(NativeTypeHandler<RemoveComponentObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, Component*>();
    }

    void InitSceneEditorAction()
    {
        Registry::Type<OpenSceneAction>();
        Registry::Type<CreateSceneObjectAction>();
        Registry::Type<DestroySceneObjectAction>();
        Registry::Type<RenameSceneObjectAction>();
        Registry::Type<AddComponentSceneObjectAction>();
        Registry::Type<UpdateComponentSceneObjectAction>();
        Registry::Type<RemoveComponentObjectAction>();
    }
}
