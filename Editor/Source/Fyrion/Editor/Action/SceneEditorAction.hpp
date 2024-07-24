#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"


namespace Fyrion
{
    class SceneEditor;
    class SceneObjectAsset;

    struct OpenSceneAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor&      sceneEditor;
        SceneObjectAsset* oldScene;
        SceneObjectAsset* newScene;

        OpenSceneAction(SceneEditor& sceneEditor, SceneObjectAsset* scene);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<OpenSceneAction>& type);
    };

    struct CreateSceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor&      sceneEditor;
        SceneObject*      parent;
        SceneObject*      current;
        SceneObjectAsset* prototype;
        usize             pos;

        CreateSceneObjectAction(SceneEditor& sceneEditor, SceneObject* parent, SceneObjectAsset* prototype);
        ~CreateSceneObjectAction() override;

        static void RegisterType(NativeTypeHandler<CreateSceneObjectAction>& type);

        void Commit() override;
        void Rollback() override;
    };


    struct DestroySceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObject* object;
        SceneObject* parent;
        usize        pos;

        DestroySceneObjectAction(SceneEditor& sceneEditor, SceneObject* object);
        ~DestroySceneObjectAction() override;

        static void RegisterType(NativeTypeHandler<DestroySceneObjectAction>& type);

        void Commit() override;
        void Rollback() override;
    };


    struct RenameSceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObject* object;
        String       newName;
        String       oldName;

        RenameSceneObjectAction(SceneEditor& sceneEditor, SceneObject* object, StringView newName);

        static void RegisterType(NativeTypeHandler<RenameSceneObjectAction>& type);

        void Commit() override;
        void Rollback() override;
    };

    struct AddComponentSceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObject* object;
        TypeHandler* typeHandler;
        Component*   component;

        AddComponentSceneObjectAction(SceneEditor& sceneEditor, SceneObject* object, TypeHandler* typeHandler);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<AddComponentSceneObjectAction>& type);
    };

    struct UpdateComponentSceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        Component*   component;
        String       currentStrValue;
        String       newStrValue;

        UpdateComponentSceneObjectAction(SceneEditor& sceneEditor, Component* component, Component* newValue);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<UpdateComponentSceneObjectAction>& type);
    };

    struct RemoveComponentObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        Component*   component;
        TypeHandler* typeHandler;
        SceneObject* object;
        String       value;

        RemoveComponentObjectAction(SceneEditor& sceneEditor, SceneObject* object, Component* component);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<RemoveComponentObjectAction>& type);
    };

    struct OverridePrototypeComponentAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObject* object;
        Component*   component;

        OverridePrototypeComponentAction(SceneEditor& sceneEditor, SceneObject* object, Component* component) : sceneEditor(sceneEditor), object(object), component(component) {}

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<OverridePrototypeComponentAction>& type);
    };

    struct RemoveOverridePrototypeComponentAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObject* object;
        Component*   component;
        String       value;

        RemoveOverridePrototypeComponentAction(SceneEditor& sceneEditor, SceneObject* object, Component* component) : sceneEditor(sceneEditor), object(object), component(component) {}

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<RemoveOverridePrototypeComponentAction>& type);
    };
}
