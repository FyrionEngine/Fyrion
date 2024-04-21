#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Scene/SceneObject.hpp"

namespace Fyrion
{
    class FY_API SceneEditor final
    {
    public:
        SceneEditor();
        ~SceneEditor();
        SceneEditor(const SceneEditor& other) = delete;
        SceneEditor(SceneEditor&& other) noexcept = default;

        void         LoadScene(RID rid);
        bool         IsLoaded() const;
        SceneObject* GetRootObject() const;
        SceneObject* FindObjectByRID(RID rid) const;
        void         CreateObject();
        void         DestroySelectedObjects();
        void         ClearSelection();
        void         SelectObject(SceneObject* object);
        bool         IsSelected(SceneObject* object);
        bool         IsParentOfSelected(SceneObject* object) const;
        bool         IsSimulating();
        SceneObject* GetLastSelectedObject() const;

    private:
        SceneObject* m_rootObject{};
        HashSet<RID> m_selectedObjects{};
        SceneObject* m_lastSelectedObject{};

        void         UpdateSceneObject(SceneObject& object, ResourceObject& resource, bool updateChildren = true);
        static void  SceneObjectAssetChanged(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldObject, ResourceObject& newObject);
    };
}
