#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"


namespace Fyrion
{
    class SceneEditor;
    class SceneObjectAsset;

    struct OpenSceneAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObjectAsset* oldScene;
        SceneObjectAsset* newScene;

        OpenSceneAction(SceneEditor& sceneEditor,  SceneObjectAsset* scene);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<OpenSceneAction>& type);
    };

    struct CreateSceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor& sceneEditor;
        SceneObjectAsset* parent;
        SceneObjectAsset* current;
        usize             pos;

        CreateSceneObjectAction(SceneEditor& sceneEditor, SceneObjectAsset* parent);
        ~CreateSceneObjectAction() override;

        static void RegisterType(NativeTypeHandler<CreateSceneObjectAction>& type);

        void Commit() override;
        void Rollback() override;
    };


    struct DestroySceneObjectAction :  EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneObjectAsset* object;
        SceneObjectAsset* parent;
        usize             pos;

        DestroySceneObjectAction(SceneObjectAsset* object);
        ~DestroySceneObjectAction() override;

        static void RegisterType(NativeTypeHandler<DestroySceneObjectAction>& type);

        void Commit() override;
        void Rollback() override;
    };
}
