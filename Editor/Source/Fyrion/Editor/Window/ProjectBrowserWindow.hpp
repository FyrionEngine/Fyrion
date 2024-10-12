#pragma once
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"


namespace Fyrion
{
    struct AssetFile;

    class FY_API ProjectBrowserWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);

        void Init(u32 id, VoidPtr userData) override;
        void Draw(u32 id, bool& open) override;
        ~ProjectBrowserWindow() override;

        static void OpenProjectBrowser(const MenuItemEventData& eventData);
        static void AddMenuItem(const MenuItemCreation& menuItem);
        static void RegisterType(NativeTypeHandler<ProjectBrowserWindow>& type);

    private:
        static MenuItemContext menuItemContext;

        String                searchString;
        f32                   contentBrowserZoom = 1.0; //TODO - save in some local setting
        AssetFile*            openDirectory = nullptr;
        String                stringCache;
        HashSet<AssetFile*>   selectedItems;
        AssetFile*            lastSelectedItem = nullptr;
        AssetFile*            renamingItem = nullptr;
        HashMap<String, bool> openTreeFolders{};


        Texture folderTexture = {};
        Texture fileTexture = {};
        Texture brickTexture = {};

        void DrawPathItems();
        void DrawTreeNode(AssetFile* assetFile);
        void SetOpenDirectory(AssetFile* directory);


        static bool CheckSelectedAsset(const MenuItemEventData& eventData);
        static void AssetRename(const MenuItemEventData& eventData);
        static void Shutdown();
        static void AssetNewFolder(const MenuItemEventData& eventData);
        static void AssetNewScene(const MenuItemEventData& eventData);
        static void AssetNewMaterial(const MenuItemEventData& eventData);
        static void AssetDelete(const MenuItemEventData& eventData);
        static void AssetShowInExplorer(const MenuItemEventData& eventData);
        static void AssetCopyPathToClipboard(const MenuItemEventData& eventData);
        static void AssetNewResourceGraph(const MenuItemEventData& eventData);
        static void AssetNewRenderGraph(const MenuItemEventData& eventData);
        static bool CheckCanReimport(const MenuItemEventData& eventData);
        static void AssetReimport(const MenuItemEventData& eventData);
        static void AssetNew(const MenuItemEventData& eventData);
    };
}
