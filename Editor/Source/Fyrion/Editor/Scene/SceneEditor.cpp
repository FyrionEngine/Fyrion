#include "SceneEditor.hpp"

#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Scene/SceneTypes.hpp"

namespace Fyrion
{
    SceneEditor::SceneEditor()
    {
        Repository::AddResourceTypeEvent(GetTypeID<SceneObjectAsset>(), this, ResourceEventType::Update, SceneObjectAssetChanged);
    }

    SceneEditor::~SceneEditor()
    {
        Repository::RemoveResourceTypeEvent(GetTypeID<SceneObjectAsset>(), this, SceneObjectAssetChanged);
    }

    void SceneEditor::LoadScene(RID rid)
    {
        m_nodes.Clear();
        m_rootNode = nullptr;

        ResourceObject asset = Repository::Read(rid);
        if (asset.Has(Asset::Object))
        {
            m_rootNode = LoadSceneObjectAsset(asset.GetSubObject(Asset::Object));
            m_rootNode->name = asset[Asset::Name].As<String>();
        }
    }

    SceneObjectNode* SceneEditor::GetRootNode() const
    {
        return m_rootNode;
    }

    bool SceneEditor::IsLoaded() const
    {
        return m_rootNode != nullptr;
    }

    SceneObjectNode* SceneEditor::LoadSceneObjectAsset(RID rid)
    {
        SceneObjectNode* node = m_nodes.Emplace(rid, MakeUnique<SceneObjectNode>()).first->second.Get();
        ResourceObject scene = Repository::Read(rid);

        node->rid = rid;
        if (scene.Has(SceneObjectAsset::Name))
        {
            node->name = scene[SceneObjectAsset::Name].As<String>();
        }

        Array<RID> children = scene.GetSubObjectSetAsArray(SceneObjectAsset::Children);
        for (RID child : children)
        {
            SceneObjectNode* childNode = LoadSceneObjectAsset(child);
            childNode->parent = node;
            node->children.EmplaceBack(childNode);
        }

        //TODO sort node->children.

        return node;
    }

    void SceneEditor::SceneObjectAssetChanged(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldObject, ResourceObject& newObject)
    {

    }
}
