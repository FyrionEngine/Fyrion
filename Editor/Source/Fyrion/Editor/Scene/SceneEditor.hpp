#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    struct SceneObjectNode
    {
        RID                     rid{};
        String                  name{};
        SceneObjectNode*        parent{};
        Array<SceneObjectNode*> children{};
    };

    class FY_API SceneEditor final
    {
    public:
        SceneEditor();
        ~SceneEditor();
        SceneEditor(const SceneEditor& other) = delete;
        SceneEditor(SceneEditor&& other) noexcept = default;

        void             LoadScene(RID rid);
        bool             IsLoaded() const;
        SceneObjectNode* GetRootNode() const;

    private:
        SceneObjectNode* m_rootNode{};
        HashMap<RID, UniquePtr<SceneObjectNode>> m_nodes{};

        SceneObjectNode* LoadSceneObjectAsset(RID rid);
        static void SceneObjectAssetChanged(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldObject, ResourceObject& newObject);
    };
}
