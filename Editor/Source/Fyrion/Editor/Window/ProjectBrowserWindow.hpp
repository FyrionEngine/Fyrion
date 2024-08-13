#pragma once

#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"


namespace Fyrion
{
    class DirectoryAsset;
    class AssetInfo;
    struct UUID;
    class Asset;

    class FY_API ProjectBrowserWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);

        void Init(u32 id, VoidPtr userData) override;
        void Draw(u32 id, bool& open) override;
        void SetOpenDirectory(DirectoryAsset* directory);
        void SetSelectedAsset(AssetInfo* selectedItem);

        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type);

    private:
        u32                            windowId{};
        DirectoryAsset*                openDirectory{};
        AssetInfo*                     selectedItem{};
        String                         searchString{};
        String                         stringCache{};
        AssetPayload                   assetPayload{};
        DirectoryAsset*                popupFolder{};
        HashMap<DirectoryAsset*, bool> openTreeFolders{};
        f32                            contentBrowserZoom = 0.8;
        Array<DirectoryAsset*>         directoryCache;
        EventHandler<OnAssetSelection> onAssetSelectionHandler{};

        inline static DirectoryAsset* lastOpenedDirectory = nullptr;

        Texture folderTexture;
        Texture fileTexture;

        void DrawDirectoryTreeNode(DirectoryAsset* directory);
        void DrawPathItems();

        static MenuItemContext menuItemContext;
        static void            Shutdown();
        static void            DropFileCallback(Window window, const StringView& path);
        static void            OpenProjectBrowser(const MenuItemEventData& eventData);

        static void AssetNewFolder(const MenuItemEventData& eventData);
        static void AssetNewScene(const MenuItemEventData& eventData);
        static void AssetNewMaterial(const MenuItemEventData& eventData);
        static void AssetDelete(const MenuItemEventData& eventData);
        static bool CheckSelectedAsset(const MenuItemEventData& eventData);
        static void AssetRename(const MenuItemEventData& eventData);
        static void AssetShowInExplorer(const MenuItemEventData& eventData);
        static void AssetCopyPathToClipboard(const MenuItemEventData& eventData);
        static void AssetNewResourceGraph(const MenuItemEventData& eventData);
        static void AssetNewRenderGraph(const MenuItemEventData& eventData);
        static bool CheckCanReimport(const MenuItemEventData& eventData);
        static void AssetReimport(const MenuItemEventData& eventData);

        static void AssetNew(const MenuItemEventData& eventData);

        void NewAsset(TypeID typeId);
    };
}
