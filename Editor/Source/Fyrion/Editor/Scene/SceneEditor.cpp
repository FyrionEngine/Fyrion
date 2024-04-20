#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
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
        m_count = 0;

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

    SceneObjectNode* SceneEditor::FindNodeByRID(RID rid) const
    {
        if (auto it = m_nodes.Find(rid))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    bool SceneEditor::IsLoaded() const
    {
        return m_rootNode != nullptr;
    }

    void SceneEditor::CreateObject()
    {
        if (m_rootNode == nullptr) return;

        if (m_selectedObjects.Empty())
        {
            RID entityRID = Repository::CreateResource<SceneObjectAsset>();

            ResourceObject root = Repository::Write(m_rootNode->rid);
            u64 size = root.GetSubObjectSetCount(SceneObjectAsset::Children);

            ResourceObject write = Repository::Write(entityRID);
            write[SceneObjectAsset::Name] = "Object " + ToString(m_count);
            write[SceneObjectAsset::Order] = size;
            write.Commit();

            Repository::SetUUID(entityRID, UUID::RandomUUID());

            root.AddToSubObjectSet(SceneObjectAsset::Children, entityRID);
            root.Commit();
        }
        else
        {
            HashSet<RID> selected = m_selectedObjects;
            ClearSelection();

            for (auto it: selected)
            {
                if (const auto& itNode = m_nodes.Find(it.first))
                {
                    SceneObjectNode* parent = itNode->second.Get();
                    ResourceObject writeParent = Repository::Write(parent->rid);
                    u64 size = writeParent.GetSubObjectSetCount(SceneObjectAsset::Children);

                    RID objectRid = Repository::CreateResource<SceneObjectAsset>();
                    m_selectedObjects.Emplace(objectRid);

                    ResourceObject write = Repository::Write(objectRid);
                    write[SceneObjectAsset::Name] = "Object " + ToString(m_count);
                    write[SceneObjectAsset::Order] = size;
                    write[SceneObjectAsset::Parent] = parent->rid;
                    write.Commit();

                    Repository::SetUUID(objectRid, UUID::RandomUUID());

                    writeParent.AddToSubObjectSet(SceneObjectAsset::Children, objectRid);
                    writeParent.Commit();
                }
            }
        }

        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::DestroySelectedObjects()
    {
        if (m_rootNode == nullptr) return;

        for (auto it: m_selectedObjects)
        {
            if (const auto& itObject = m_nodes.Find(it.first))
            {
                if (itObject->second->rid)
                {
                    Repository::DestroyResource(itObject->second->rid);
                }
            }
        }

        m_selectedObjects.Clear();
        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::ClearSelection()
    {
        for (auto& it : m_selectedObjects)
        {
            if (const auto& itObject = m_nodes.Find(it.first))
            {
                itObject->second->selected = false;
            }
        }
        m_selectedObjects.Clear();
        m_lastSelectedObject = nullptr;
    }

    void SceneEditor::SelectObject(SceneObjectNode* node)
    {
        node->selected = true;
        m_selectedObjects.Emplace(node->rid);
        m_lastSelectedObject = node;
    }

    bool SceneEditor::IsParentOfSelected(SceneObjectNode* node) const
    {
        for (auto& it : m_selectedObjects)
        {
            if (const auto& itObject = m_nodes.Find(it.first))
            {
                if (itObject->second->parent == node)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    SceneObjectNode* SceneEditor::GetLastSelectedObject() const
    {
        return m_lastSelectedObject;
    }

    SceneObjectNode* SceneEditor::LoadSceneObjectAsset(RID rid)
    {
        SceneObjectNode* node = m_nodes.Emplace(rid, MakeUnique<SceneObjectNode>(rid)).first->second.Get();
        ResourceObject scene = Repository::Read(rid);
        UpdateSceneObjectNode(*node, scene);
        m_count++;
        return node;
    }

    void SceneEditor::UpdateSceneObjectNode(SceneObjectNode& node, ResourceObject& object, bool updateChildren)
    {
        node.children.Clear();
        if (object.Has(SceneObjectAsset::Name))
        {
            node.name = object[SceneObjectAsset::Name].As<String>();
        }

        if (object.Has(SceneObjectAsset::Order))
        {
            node.order = object[SceneObjectAsset::Order].As<u64>();
        }

        if (m_selectedObjects.Has(node.rid) && !node.selected)
        {
            node.selected = true;
        }

        node.uuid = Repository::GetUUID(object.GetRID());

        Array<RID> children = object.GetSubObjectSetAsArray(SceneObjectAsset::Children);
        for (RID child : children)
        {
            SceneObjectNode* childNode = FindNodeByRID(child);
            if (!childNode || updateChildren)
            {
                childNode = LoadSceneObjectAsset(child);
            }
            childNode->parent = &node;
            node.children.EmplaceBack(childNode);
        }

        Sort(node.children.begin(), node.children.end(), [](SceneObjectNode* left, SceneObjectNode* right)
        {
            return left->order < right->order;
        });
    }

    void SceneEditor::SceneObjectAssetChanged(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldObject, ResourceObject& newObject)
    {
        SceneEditor* sceneEditor = static_cast<SceneEditor*>(userData);
        SceneObjectNode* node = sceneEditor->FindNodeByRID(newObject.GetRID());
        if (node)
        {
            sceneEditor->UpdateSceneObjectNode(*node, newObject, false);
        }
    }
}
