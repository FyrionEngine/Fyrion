#include "SceneEditorAction.hpp"

#include "Fyrion/Asset/AssetSerialization.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Editor/SceneEditor.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/Scene/SceneManager.hpp"
#include "Fyrion/Scene/Components/TransformComponent.hpp"

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

    CreateSceneObjectAction::CreateSceneObjectAction(SceneEditor& sceneEditor, SceneObject* parent, SceneObjectAsset* prototype) : sceneEditor(sceneEditor), parent(parent), prototype(prototype)
    {
        current = SceneManager::CreateObjectFromAsset(prototype);
        if (current->GetName().Empty())
        {
            current->SetName("New Object");
        }
        current->SetUUID(UUID::RandomUUID());
        pos = parent->GetChildren().Size();
    }

    CreateSceneObjectAction::~CreateSceneObjectAction()
    {
        if (current != nullptr && current->GetParent() == nullptr)
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
        type.Constructor<SceneEditor, SceneObject*, SceneObjectAsset*>();
    }

    DestroySceneObjectAction::DestroySceneObjectAction(SceneEditor& sceneEditor, SceneObject* object) : sceneEditor(sceneEditor), object(object), parent(object->GetParent()) {}

    DestroySceneObjectAction::~DestroySceneObjectAction()
    {
        if (object && object->GetParent() == nullptr)
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

    AddComponentSceneObjectAction::AddComponentSceneObjectAction(SceneEditor& sceneEditor, SceneObject* object, TypeHandler* typeHandler)
        : sceneEditor(sceneEditor),
          object(object),
          typeHandler(typeHandler)
    {
        component = typeHandler->Cast<Component>(typeHandler->NewInstance());
        component->typeHandler = typeHandler;
        component->SetUUID(UUID::RandomUUID());
    }

    void AddComponentSceneObjectAction::Commit()
    {
        object->AddComponent(component);
        sceneEditor.Modify();
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
        JsonAssetWriter writer(SerializationOptions::IncludeNullOrEmptyValues);
        currentStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(component->typeHandler, writer, component));
        newStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(component->typeHandler, writer, newValue));

        JsonAssetReader reader(newStrValue);
        Serialization::Deserialize(component->typeHandler, reader, reader.ReadObject(), component);

        sceneEditor.Modify();
        component->OnChange();

        ImGui::ClearDrawData(component, false);
    }

    void UpdateComponentSceneObjectAction::Commit()
    {
        JsonAssetReader reader(newStrValue);
        Serialization::Deserialize(component->typeHandler, reader, reader.ReadObject(), component);

        ImGui::ClearDrawData(component);

        sceneEditor.Modify();
        component->OnChange();
    }

    void UpdateComponentSceneObjectAction::Rollback()
    {
        JsonAssetReader reader(currentStrValue);
        Serialization::Deserialize(component->typeHandler, reader, reader.ReadObject(), component);

        ImGui::ClearDrawData(component);

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
        value = JsonAssetWriter::Stringify(Serialization::Serialize(typeHandler, writer, component));
        object->RemoveComponent(component);
        sceneEditor.Modify();
        typeHandler->Destroy(component);
    }

    void RemoveComponentObjectAction::Rollback()
    {
        component = &object->CreateComponent(typeHandler);

        JsonAssetReader reader(value);
        Serialization::Deserialize(typeHandler, reader, reader.ReadObject(), component);
        component->OnChange();
        sceneEditor.Modify();
    }

    void RemoveComponentObjectAction::RegisterType(NativeTypeHandler<RemoveComponentObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, Component*>();
    }

    void OverridePrototypeComponentAction::Commit()
    {
        object->OverridePrototypeComponent(component);
        sceneEditor.Modify();
    }

    void OverridePrototypeComponentAction::Rollback()
    {
        object->RemoveOverridePrototypeComponent(component);
        sceneEditor.Modify();
    }

    void OverridePrototypeComponentAction::RegisterType(NativeTypeHandler<OverridePrototypeComponentAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, Component*>();
    }

    void RemoveOverridePrototypeComponentAction::Commit()
    {
        JsonAssetWriter writer;
        value = JsonAssetWriter::Stringify(Serialization::Serialize(component->typeHandler, writer, component));

        object->RemoveOverridePrototypeComponent(component);
        sceneEditor.Modify();

        ImGui::ClearDrawData(component);
    }

    void RemoveOverridePrototypeComponentAction::Rollback()
    {
        object->OverridePrototypeComponent(component);

        JsonAssetReader reader(value);
        Serialization::Deserialize(component->typeHandler, reader, reader.ReadObject(), component);
        component->OnChange();

        sceneEditor.Modify();

        ImGui::ClearDrawData(component);
    }

    void RemoveOverridePrototypeComponentAction::RegisterType(NativeTypeHandler<RemoveOverridePrototypeComponentAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, Component*>();
    }

    MoveTransformObjectAction::MoveTransformObjectAction(SceneEditor& sceneEditor, SceneObject* object, TransformComponent* transformComponent, Transform& oldTransform)
        : sceneEditor(sceneEditor),
          object(object),
          transformComponent(transformComponent),
          oldTransform(oldTransform)
    {
        newTransform = transformComponent->GetTransform();
    }

    void MoveTransformObjectAction::Commit()
    {
        transformComponent->SetTransform(newTransform);
        sceneEditor.Modify();

        ImGui::ClearDrawData(transformComponent);
    }

    void MoveTransformObjectAction::Rollback()
    {
        transformComponent->SetTransform(oldTransform);
        sceneEditor.Modify();

        ImGui::ClearDrawData(transformComponent);
    }

    void MoveTransformObjectAction::RegisterType(NativeTypeHandler<MoveTransformObjectAction>& type)
    {
        type.Constructor<SceneEditor, SceneObject*, TransformComponent*, Transform>();
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
        Registry::Type<OverridePrototypeComponentAction>();
        Registry::Type<RemoveOverridePrototypeComponentAction>();
        Registry::Type<MoveTransformObjectAction>();
    }
}
