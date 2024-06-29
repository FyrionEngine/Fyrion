#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"

namespace Fyrion
{
    class FY_API SceneEditor
    {
    public:
        SceneObjectAsset* GetRootObject() const;
        void              ClearSelection();
        void              SelectObject(SceneObjectAsset& object);
        void              DeselectObject(SceneObjectAsset& object);
        bool              IsSelected(SceneObjectAsset& object) const;
        bool              IsParentOfSelected(SceneObjectAsset& object) const;
        void              RenameObject(SceneObjectAsset& asset, StringView newName);
        void              DestroySelectedObjects();
        void              CreateObject();
        bool              IsSimulating();
        void              LoadScene(SceneObjectAsset* asset);
    private:
        SceneObjectAsset* root = nullptr;
        HashSet<usize>    selectedObjects{};

        static void ClearSelectionStatic(VoidPtr userData);
    };
}
