#include "AssetEditor.hpp"

#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    bool AssetFile::IsDirty() const
    {
        return currentVersion > persistedVersion;
    }


    void AssetEditor::AddPackage(StringView directory)
    {
        AssetFile* assetFile = MemoryGlobals::GetDefaultAllocator().Alloc<AssetFile>();
        directories.EmplaceBack(assetFile);
        UpdateCache(directory, assetFile);
    }

    const AssetFile* AssetEditor::FindNode(StringView path) const
    {
        if (auto it = assets.Find(path))
        {
            return it->second;
        }
        return nullptr;
    }

    String AssetEditor::CreateDirectory(StringView parentPath)
    {
        if (auto it = assets.Find(parentPath))
        {
            AssetFile* parent = it->second;

            AssetFile* newDirectory = MemoryGlobals::GetDefaultAllocator().Alloc<AssetFile>();

            newDirectory->fileName = "New Folder"; //TODO generate name
            newDirectory->absolutePath = Path::Join(parentPath, newDirectory->fileName);
            newDirectory->hash = HashInt32(HashValue(newDirectory->absolutePath));
            newDirectory->isDirectory = true;
            newDirectory->currentVersion = 1;
            newDirectory->persistedVersion = 0;
            assets.Insert(newDirectory->absolutePath, newDirectory);

            parent->children.EmplaceBack(newDirectory);

            return newDirectory->absolutePath;
        }
        return {};
    }

    void AssetEditor::Rename(AssetFile* assetFile, StringView newName)
    {
        assetFile->fileName = newName;
        assetFile->currentVersion++;
    }

    void AssetEditor::GetUpdatedAssets(Array<AssetFile*>& updatedAssets) const
    {
        for (auto& it : assets)
        {
            if (it.second->IsDirty())
            {
                updatedAssets.EmplaceBack(it.second);
            }
        }
    }

    void AssetEditor::SaveAssets(Span<AssetFile*> assetsToSave)
    {
        for (AssetFile* assetFile : assetsToSave)
        {
            assets.Erase(assetFile->absolutePath);
            String newAbsolutePath = Path::Join(Path::Parent(assetFile->absolutePath), assetFile->fileName, assetFile->extension);

            if (assetFile->isDirectory)
            {
                if (FileSystem::GetFileStatus(assetFile->absolutePath).exists)
                {
                    FileSystem::Rename(assetFile->absolutePath, newAbsolutePath);
                }
                else
                {
                    FileSystem::CreateDirectory(newAbsolutePath);
                }
            }

            assetFile->absolutePath = newAbsolutePath;
            assets.Insert(newAbsolutePath, assetFile);
            assetFile->persistedVersion = assetFile->currentVersion;
        }
    }

    AssetEditor::~AssetEditor()
    {
        for(auto& it: assets)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(it.second);
        }
    }

    void AssetEditor::UpdateCache(StringView path, AssetFile* assetFile)
    {
        assets.Insert(path, assetFile);

        assetFile->children.Clear();

        FileStatus status = FileSystem::GetFileStatus(path);

        assetFile->absolutePath = path;
        assetFile->fileName = Path::Name(path);
        assetFile->extension = Path::Extension(path);
        assetFile->hash = HashInt32(HashValue(path));
        assetFile->isDirectory = status.isDirectory;
        assetFile->currentVersion = 1;
        assetFile->persistedVersion = 1;

        for (const String& child : DirectoryEntries{path})
        {
            AssetFile* childFile = MemoryGlobals::GetDefaultAllocator().Alloc<AssetFile>();
            assetFile->children.EmplaceBack(childFile);
            UpdateCache(child, childFile);
        }
    }
}
