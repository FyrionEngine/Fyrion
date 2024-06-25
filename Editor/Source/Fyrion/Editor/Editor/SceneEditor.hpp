#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"

namespace Fyrion
{
    class FY_API SceneEditor
    {
    public:
        SceneObjectAsset* GetRootObject() const;
        void ClearSelection();
        void SelectObject(SceneObjectAsset& object);
        bool IsSelected(SceneObjectAsset& object) const;
        bool IsParentOfSelected(SceneObjectAsset& object) const;
        void DestroySelectedObjects();
        void CreateObject();
        bool IsSimulating();
        void LoadScene(SceneObjectAsset* asset);
    private:
        SceneObjectAsset* root;
    };
}
