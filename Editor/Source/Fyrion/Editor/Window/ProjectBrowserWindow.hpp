#pragma once

#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class FY_API ProjectBrowserWindow : public EditorWindow
    {
    public:
        ProjectBrowserWindow();

        void Init(u32 id, VoidPtr userData) override;
        void Draw(u32 id, bool& open) override;
        void SetOpenFolder(RID folder);

        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type);

    private:
        AssetTree&         m_assetTree;
        u32                m_windowId{};
        RID                m_openFolder{};
        RID                m_selectedItem{};
        String             m_searchString{};
        String             m_stringCache{};
        RID                m_movingItem{};
        Array<AssetNode*>  m_folderCache{};
        RID                m_popupFolder{};
        HashMap<RID, bool> m_openTreeFolders{};
        f32                m_contentBrowserZoom = 0.8;

        void DrawTreeNode(AssetNode* node);
        void DrawPathItems();

        static MenuItemContext s_menuItemContext;
        static void Shutdown();
        static void OpenProjectBrowser(VoidPtr userData);

        static void AssetNewFolder(VoidPtr userData);
        static void AssetNewScene(VoidPtr userData);
        static void AssetDelete(VoidPtr userData);
        static bool CheckSelectedAsset(VoidPtr userData);
        static void AssetRename(VoidPtr userData);
        static void AssetShowInExplorer(VoidPtr userData);
        static void AssetNewResourceGraph(VoidPtr userData);
    };

}