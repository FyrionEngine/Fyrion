#pragma once
#include "AssetTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/IO/Asset.hpp"


namespace Fyrion
{
    struct AssetFile : AssetLoader
    {
        u32    hash;
        String fileName;
        String extension;
        String absolutePath;
        String tempBuffer;
        bool   isDirectory;
        UUID   uuid;

        u64 currentVersion;
        u64 persistedVersion;

        Array<AssetFile*> children;

        AssetFile* parent;

        AssetHandler* handler;

        bool active = true;

        bool IsDirty() const;
        void RemoveFromParent();

        Texture thumbnail = {};
        bool    thumbnailVerified = false;

        Asset*    LoadAsset() override;
        Array<u8> LoadStream(usize offset, usize size) override;

        OutputFileStream CreateStream();

        Texture GetThumbnail();

        void MoveTo(AssetFile* newParent);
        bool IsChildOf(AssetFile* item) const;
        void Destroy();

        ~AssetFile() override;
    };


    namespace AssetEditor
    {
        FY_API void             AddPackage(StringView name, StringView directory);
        FY_API void             SetProject(StringView name, StringView directory);
        FY_API Span<AssetFile*> GetPackages();
        FY_API AssetFile*       GetProject();
        FY_API AssetFile*       CreateDirectory(AssetFile* parent);
        FY_API AssetFile*       CreateAsset(AssetFile* parent, TypeID typeId, StringView suggestedName = "");
        FY_API void             Rename(AssetFile* assetFile, StringView newName);
        FY_API void             GetUpdatedAssets(Array<AssetFile*>& updatedAssets);
        FY_API void             SaveAssets(Span<AssetFile*> assetsToSave);
        FY_API void             DeleteAssets(Span<AssetFile*> assetFile);
        FY_API String           CreateUniqueName(AssetFile* parent, StringView desiredName);
        FY_API void             ImportAssets(AssetFile* parent, Span<String> paths);
        FY_API void             FilterExtensions(Array<FileFilter>& extensions);
    }
}
