#pragma once
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Editor/MenuItem.hpp"
#include "Fyrion/Editor/Asset/FileTreeCache.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"


namespace Fyrion
{
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
        f32                   contentBrowserZoom = 0.8;
        String                openDirectory;
        String                stringCache;
        HashMap<String, bool> openTreeFolders{};

        FileTreeCache fileTreeCache;

        Texture folderTexture = {};
        Texture fileTexture = {};

        void DrawPathItems();
        void DrawTreeNode(const FileTreeCacheNode& node);
        void SetOpenDirectory(StringView directory);
    };
}
