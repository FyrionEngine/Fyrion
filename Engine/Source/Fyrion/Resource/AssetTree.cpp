#include <cctype>
#include "AssetTree.hpp"
#include "Repository.hpp"
#include "ResourceAssets.hpp"

namespace Fyrion
{

    //TODO move to Algorithm.h with a custom toupper(char) function
    inline void ToUpper(char* begin, char* end)
    {
        for (char* it = begin; it != end; ++it)
        {
            *it = toupper(*it);
        }
    }

    SharedPtr<AssetNode> AssetTree::MakeAssetNode(RID rid, RID root)
    {
        SharedPtr<AssetNode> node = MakeShared<AssetNode>(rid, root, Repository::GetResourceTypeID(rid), ResourceAssets::GetName(rid));
        node->updated = Repository::GetVersion(rid) > ResourceAssets::GetLoadedVersion(rid);
        node->active = Repository::IsActive(rid);
        node->path = ResourceAssets::GetPath(rid);
        if (node->type == GetTypeID<Asset>())
        {
            ResourceObject asset = Repository::Read(rid);
            if (asset.Has(Asset::Object))
            {
                RID subobject = asset.GetSubObject(Asset::Object);
                node->assetDesc = Repository::GetResourceTypeSimpleName(Repository::GetResourceType(subobject));
            }
        }
        return node;
    }

    AssetNode* AssetTree::GetOrCreateNode(RID root, RID rid)
    {
        auto it = m_nodes.Find(rid);
        if (it == m_nodes.end())
        {
            it = m_nodes.Emplace(rid, MakeAssetNode(rid, root)).first;
        }
        return it->second.Get();
    }

    void AssetTree::AddAssetRoot(RID root)
    {
        FY_ASSERT(Repository::GetResourceTypeID(root) == GetTypeID<AssetRoot>(), "rid is not AssetRoot");
        m_rootNodeIds.EmplaceBack(root);
        m_dirty = true;
    }

    AssetNode* AssetTree::GetNode(RID rid)
    {
        auto it = m_nodes.Find(rid);
        if (it != m_nodes.end())
        {
            return it->second.Get();
        }
        return nullptr;
    }

    void AssetTree::SortNodes(AssetTreeSort sort, bool desc)
    {
        m_sort = sort;
        m_sortDesc = desc;
        for (AssetNode* it: m_rootNodes)
        {
            SortNodes(it);
        }
    }

    void AssetTree::SortNodes(AssetNode* nodes)
    {
        Sort(nodes->nodes.begin(), nodes->nodes.end(), [&](AssetNode* left, AssetNode* right)
        {
            String leftName = left->name;
            String rightName = right->name;

            ToUpper(leftName.begin(), leftName.end());
            ToUpper(rightName.begin(), rightName.end());

            return m_sortDesc ? leftName < rightName : leftName > rightName;
        });

        ForEach(nodes->nodes.begin(), nodes->nodes.end(), [&](AssetNode* node)
        {
            SortNodes(node);
        });
    }

    Span<AssetNode*> AssetTree::GetRootNodes()
    {
        return m_rootNodes;
    }

    Span<RID> AssetTree::GetAssetRoots() const
    {
        return m_rootNodeIds;
    }

    void AssetTree::GetUpdated(Array<RID>& updatedItems)
    {
        for (AssetNode* root: m_rootNodes)
        {
            for (AssetNode* child: root->nodes)
            {
                GetUpdated(child, updatedItems);
            }
        }
    }

    void AssetTree::GetUpdated(AssetNode* node, Array<RID>& updatedItems)
    {
        if (node == nullptr) return;

        if (node->updated || !node->active)
        {
            updatedItems.EmplaceBack(node->rid);
        }

        for (AssetNode* child: node->nodes)
        {
            GetUpdated(child, updatedItems);
        }
    }

    void AssetTree::Update()
    {
        if (!m_dirty) return;

        m_rootNodes.Clear();
        m_nodes.Clear();

        for (RID root: m_rootNodeIds)
        {

            auto itRoot = m_nodes.Find(root);
            if (itRoot == m_nodes.end())
            {
                itRoot = m_nodes.Emplace(root, MakeAssetNode(root, root)).first;
                itRoot->second->path = itRoot->second->name + ":/";
            }
            m_rootNodes.EmplaceBack(itRoot->second.Get());

            ResourceObject assetRoot = Repository::Read(root);

            Array<RID> dirs = assetRoot.GetSubObjectSetAsArray(AssetRoot::Directories);
            Array<RID> assets = assetRoot.GetSubObjectSetAsArray(AssetRoot::Assets);

            for (RID dir: dirs)
            {
                auto it = m_nodes.Find(dir);
                if (it == m_nodes.end())
                {
                    it = m_nodes.Emplace(dir, MakeAssetNode(dir, root)).first;
                }

                it->second->parent = GetOrCreateNode(root, ResourceAssets::GetParent(dir));
                it->second->parent->nodes.EmplaceBack(it->second.Get());
            }

            for (RID asset: assets)
            {
                auto it = m_nodes.Find(asset);
                if (it == m_nodes.end())
                {
                    it = m_nodes.Emplace(asset, MakeAssetNode(asset, root)).first;
                }
                it->second->parent = GetOrCreateNode(root, ResourceAssets::GetParent(asset));
                it->second->parent->nodes.EmplaceBack(it->second.Get());
            }

            SortNodes(itRoot->second.Get());
        }
        m_dirty = false;
    }

    String AssetTree::CreateUniqueName(AssetNode* node, const StringView& desiredName)
    {
        if (node == nullptr) return {};

        u32 count{};
        String finalName = desiredName;
        bool nameFound;
        do
        {
            nameFound = true;
            for (AssetNode* child: node->nodes)
            {
                if (finalName == child->name)
                {
                    finalName = desiredName;
                    finalName += " (";
                    finalName.Append(++count);
                    finalName += ")";
                    nameFound = false;
                    break;
                }
            }
        } while (!nameFound);
        return finalName;
    }

    RID AssetTree::NewDirectory(RID parent, const StringView& desiredName)
    {
        AssetNode* node = GetNode(parent);
        if (node == nullptr) return {};

        RID newDirectory = Repository::CreateResource<AssetDirectory>();
        String directoryName = CreateUniqueName(node, desiredName);

        ResourceObject assetDirectory = Repository::Write(newDirectory);
        assetDirectory.SetValue(AssetDirectory::Name, directoryName);
        assetDirectory.SetValue(AssetDirectory::Parent, parent);
        assetDirectory.Commit();

        ResourceObject assetRoot = Repository::Write(node->root);
        assetRoot.AddToSubObjectSet(AssetRoot::Directories, newDirectory);
        assetRoot.Commit();

        MarkDirty();

        return newDirectory;
    }

    RID AssetTree::NewAsset(RID parent, RID object, const StringView& desiredName)
    {
        AssetNode* node = GetNode(parent);
        if (node == nullptr) return {};

        RID newAsset = Repository::CreateResource<Asset>();
        ResourceObject write = Repository::Write(newAsset);
        write[Asset::Name] = CreateUniqueName(node, desiredName);
        write[Asset::Directory] = parent;
        write[Asset::Extension] = FY_ASSET_EXTENSION;
        write.SetSubObject(Asset::Object, object);
        write.Commit();

        ResourceObject assetRoot = Repository::Write(node->root);
        assetRoot.AddToSubObjectSet(AssetRoot::Assets, newAsset);
        assetRoot.Commit();

        ResourceObject asset = Repository::Write(object);
        asset.Commit();

        if (!Repository::GetUUID(object))
        {
            Repository::SetUUID(object, UUID::RandomUUID());
        }

        MarkDirty();

        return newAsset;
    }

    void AssetTree::Move(RID newDirectory, RID rid)
    {
        AssetNode* dirNode = GetNode(newDirectory);
        AssetNode* assetNode = GetNode(rid);
        if (dirNode == nullptr || assetNode == nullptr) return;

        String name = CreateUniqueName(dirNode, assetNode->name);

        if (assetNode->type == GetTypeID<Asset>())
        {
            ResourceObject asset = Repository::Write(rid);
            asset.SetValue(Asset::Directory, newDirectory);
            asset.SetValue(Asset::Name, name);
            asset.Commit();

            if (assetNode->root != dirNode->root)
            {
                ResourceObject originRoot = Repository::Write(assetNode->root);
                originRoot.RemoveFromSubObjectSet(AssetRoot::Assets, rid);
                originRoot.Commit();

                ResourceObject newRoot = Repository::Write(dirNode->root);
                newRoot.AddToSubObjectSet(AssetRoot::Assets, rid);
                newRoot.Commit();
            }
        }
        else if (assetNode->type == GetTypeID<AssetDirectory>())
        {
            ResourceObject assetDirectory = Repository::Write(rid);
            assetDirectory.SetValue(AssetDirectory::Parent, newDirectory);
            assetDirectory.SetValue(AssetDirectory::Name, name);
            assetDirectory.Commit();

            if (assetNode->root != dirNode->root)
            {
                ResourceObject originRoot = Repository::Write(assetNode->root);
                originRoot.RemoveFromSubObjectSet(AssetRoot::Directories, rid);
                originRoot.Commit();

                ResourceObject newRoot = Repository::Write(dirNode->root);
                newRoot.AddToSubObjectSet(AssetRoot::Directories, rid);
                newRoot.Commit();
            }
        }
        UpdateChildren(assetNode);
        MarkDirty();
    }

    void AssetTree::Rename(RID rid, const StringView& desiredName)
    {
        AssetNode* node = GetNode(rid);
        if (node == nullptr) return;
        if (desiredName.Empty()) return;
        if (node->name == desiredName) return;

        String name = CreateUniqueName(node->parent, desiredName);

        ResourceObject write = Repository::Write(rid);
        if (node->type == GetTypeID<AssetDirectory>())
        {
            write[AssetDirectory::Name] = name;
        }
        if (node->type == GetTypeID<Asset>())
        {
            write[Asset::Name] = name;
        }
        write.Commit();

        UpdateChildren(node);

        MarkDirty();
    }

    void AssetTree::Delete(RID rid)
    {
        AssetNode* node = GetNode(rid);
        if (node == nullptr) return;
        for (AssetNode* child: node->nodes)
        {
            Delete(child->rid);
        }

        if (ResourceAssets::GetLoadedVersion(rid) > 0)
        {
            Repository::InactiveResource(rid);
        }
        else
        {
            Repository::DestroyResource(rid);
        }

        if (!node->path.Empty())
        {
            Repository::RemovePath(node->path);
        }

        MarkDirty();
    }

    bool AssetTree::IsParentOf(RID rid, RID parent)
    {
        return IsParentOf(GetNode(rid), GetNode(parent));
    }

    bool AssetTree::IsParentOf(AssetNode* node, AssetNode* parent)
    {
        if (node == nullptr || node->parent == nullptr || parent == nullptr) return false;

        if (node->parent->rid == parent->rid)
        {
            return true;
        }
        return IsParentOf(node->parent, parent);
    }

    void AssetTree::MarkDirty()
    {
        m_dirty = true;
    }

    void AssetTree::UpdateChildren(AssetNode* node)
    {
        for (AssetNode* child: node->nodes)
        {
            ResourceObject obj = Repository::Write(child->rid);
            obj.Commit();
            UpdateChildren(child);
        }
    }

}