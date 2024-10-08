#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"

namespace Fyrion
{
    class FY_API SceneEditor
    {
    public:
        SceneObject* GetRootObject() const;
        void         Modify() const;
        void         ClearSelection();
        void         SelectObject(SceneObject& object);
        void         DeselectObject(SceneObject& object);
        bool         IsSelected(SceneObject& object) const;
        bool         IsParentOfSelected(SceneObject& object) const;
        void         RenameObject(SceneObject& asset, StringView newName);
        void         DestroySelectedObjects();
        void         CreateObject(SceneObjectAsset* prototype = nullptr);
        bool         IsRootSelected() const;
        void         AddComponent(SceneObject& object, TypeHandler* typeHandler);
        void         ResetComponent(SceneObject& object, Component* component);
        void         RemoveComponent(SceneObject& object, Component* component);
        void         UpdateComponent(SceneObject* sceneObject, Component* component, Component* newValue);
        void         OverridePrototypeComponent(SceneObject* object, Component* component);
        void         RemoveOverridePrototypeComponent(SceneObject* object, Component* component);

        const HashSet<usize>& GetSelectedObjects() const;

        void              LoadScene(SceneObjectAsset* asset);
        SceneObjectAsset* GetScene() const;

        bool IsSimulating();
        void StartSimulation();
        void StopSimulation();

    private:
        SceneObjectAsset* scene = nullptr;
        HashSet<usize>    selectedObjects{};

        EventHandler<OnSceneObjectSelection> onSceneObjectAssetSelection{};

        static void ClearSelectionStatic(VoidPtr userData);
    };
}
