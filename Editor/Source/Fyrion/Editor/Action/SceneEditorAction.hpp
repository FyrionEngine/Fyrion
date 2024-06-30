#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"


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


    enum class SceneObjectActionType
    {
        Create,
        Destroy,
        Rename
    };

    struct SceneObjectAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        SceneEditor&          sceneEditor;
        SceneObjectActionType type ;
        SceneObject*          parent = nullptr;
        SceneObject*          current = nullptr;
        usize                 pos = 0;
        String                newName = "";

        SceneObjectAction(SceneEditor& sceneEditor, SceneObjectActionType type);

        static void RegisterType(NativeTypeHandler<SceneObjectAction>& type);

        void Commit() override;
        void Rollback() override;
    };
}
