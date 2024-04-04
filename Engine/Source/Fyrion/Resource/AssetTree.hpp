#pragma once

#include "ResourceTypes.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Core/HashMap.hpp"
namespace Fyrion
{
	struct AssetNode
	{
        RID rid;
        RID root;
        TypeID type;
        String name;
        String path;
        String assetDesc;
        bool updated;
        bool active;
        AssetNode* parent;
        Array<AssetNode*> nodes;
	};

    enum class AssetTreeSort
    {
        Name = 0,
        Type = 1,
        Size = 2,
        Modified = 3,
    };


	class FY_API AssetTree
	{
	public:
		void                AddAssetRoot(RID root);
		AssetNode*          GetNode(RID rid);
		void                SortNodes(AssetTreeSort sort, bool desc = true);
		Span<AssetNode*>    GetRootNodes();
		Span<RID>           GetAssetRoots() const;
		void                GetUpdated(Array<RID>& updatedItems);
		void                Update();

		bool                IsParentOf(RID rid, RID parent);
		bool                IsParentOf(AssetNode* node, AssetNode* parent);

		RID                 NewDirectory(RID parent, const StringView& desiredName);
		RID                 NewAsset(RID parent, const StringView& desiredName);
		void                Move(RID newDirectory, RID rid);
		void                Rename(RID rid, const StringView& desiredName);
		void                Delete(RID rid);
		void                MarkDirty();
	private:
		Array<RID>                         m_rootNodeIds{};
		Array<AssetNode*>                  m_rootNodes{};
		HashMap<RID, SharedPtr<AssetNode>> m_nodes{};
		AssetTreeSort                   m_sort     = AssetTreeSort::Name;
		bool                               m_sortDesc = true;
		bool                               m_dirty    = false;

		void                    SortNodes(AssetNode* nodes);
		AssetNode*              GetOrCreateNode(RID root, RID rid);
		SharedPtr<AssetNode>    MakeAssetNode(RID rid, RID root);
		static String           CreateUniqueName(AssetNode* node, const StringView& desiredName);
		void                    GetUpdated(AssetNode* node, Array<RID>& updatedItems);
        void                    UpdateChildren(AssetNode* node);

	};
}