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
        SceneEditor() = default;
        SceneEditor(const SceneEditor& other) = delete;
        SceneEditor(SceneEditor&& other) noexcept = default;

        void         LoadScene(RID rid);
        bool         IsLoaded() const;
        RID          GetRootObject() const;
        void         CreateObject();
        void         DestroySelectedObjects();
        void         ClearSelection();
        void         SelectObject(RID object);
        bool         IsSelected(RID object);
        bool         IsParentOfSelected(RID object) const;
        bool         IsSimulating();
        SceneObject* GetLastSelectedObject() const;
        void         AddComponent(SceneObject* object, TypeHandler* typeHandler);

    private:
        RID          m_rootObject{};
        HashSet<RID> m_selectedObjects{};
        RID          m_lastSelectedRid{};
        u64          m_count{}; //TODO this count is just for creating the object names, but it doesn't work correct.
    };
}
