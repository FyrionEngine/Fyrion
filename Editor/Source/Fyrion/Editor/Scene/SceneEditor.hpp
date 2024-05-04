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

        void          LoadScene(RID rid);
        bool          IsLoaded() const;
        RID           GetRootObject() const;
        StringView    GetRootName() const;
        void          CreateObject();
        void          DestroySelectedObjects();
        void          ClearSelection();
        void          SelectObject(RID object);
        bool          IsSelected(RID object) const;
        bool          IsParentOfSelected(RID object) const;
        bool          IsSimulating();
        RID           GetLastSelectedObject() const;
        void          RenameObject(RID rid, const StringView& newName);
        void          AddComponent(RID object, TypeHandler* typeHandler);
        void          RemoveComponent(RID object, RID component);
        void          UpdateComponent(RID component, VoidPtr value);

    private:
        RID          m_rootObject{};
        RID          m_asset{};
        String       m_rootName{};
        HashSet<RID> m_selectedObjects{};
        RID          m_lastSelectedRid{};
        u64          m_count{}; //TODO this count is just for creating the object names, but it doesn't work correct.

        static u64 SubObjectCount(RID rid);
    };
}
