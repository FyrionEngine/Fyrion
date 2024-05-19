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
        static void OpenProjectBrowser(const MenuItemEventData& eventData);

        static void AssetNewFolder(const MenuItemEventData& eventData);
        static void AssetNewScene(const MenuItemEventData& eventData);
        static void AssetDelete(const MenuItemEventData& eventData);
        static bool CheckSelectedAsset(const MenuItemEventData& eventData);
        static void AssetRename(const MenuItemEventData& eventData);
        static void AssetShowInExplorer(const MenuItemEventData& eventData);
        static void AssetNewResourceGraph(const MenuItemEventData& eventData);
        static void AssetNewRenderGraph(const MenuItemEventData& eventData);
    };

}