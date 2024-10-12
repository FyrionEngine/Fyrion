#include "AssetEditor.hpp"

#include "AssetTypes.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    namespace
    {
        Array<AssetFile*>           packages;
        HashMap<String, AssetFile*> assets;

        Array<AssetImporter*>           importers;
        HashMap<String, AssetImporter*> extensionImporters;

        AssetFile* AllocateNew(StringView name)
        {
            AssetFile* assetFile = MemoryGlobals::GetDefaultAllocator().Alloc<AssetFile>();
            assetFile->fileName = name;
            assetFile->hash = HashInt32(HashValue(reinterpret_cast<usize>(assetFile)));
            assetFile->currentVersion = 1;
            return assetFile;
        }

        AssetFile* ScanForAssets(StringView path)
        {
            FileStatus status = FileSystem::GetFileStatus(path);
            if (!status.exists) return nullptr;

            //TODO check if exists in assets

            if (status.isDirectory)
            {
                AssetFile* assetFile = AllocateNew(Path::Name(path));
                assetFile->absolutePath = path;
                assetFile->isDirectory = true;
                assetFile->persistedVersion = 1;

                for (const String& child : DirectoryEntries{path})
                {
                    if (AssetFile* assetChild = ScanForAssets(child))
                    {
                        assetChild->parent = assetFile;
                        assetFile->children.EmplaceBack(assetChild);
                    }
                }
                assets.Insert(path, assetFile);
                return assetFile;
            }

            String extension = Path::Extension(path);
            if (extension == ".buffer") return nullptr;

            if (extension != ".info")
            {
                String infoFile = Path::Join(path, ".info");

                //TODO load info

                AssetFile* assetFile = AllocateNew(Path::Name(path));
                assetFile->isDirectory = false;
                assetFile->absolutePath = path;
                assetFile->extension = Path::Extension(path);
                assetFile->persistedVersion = 1;
                assets.Insert(path, assetFile);
                return assetFile;
            }

            return nullptr;
        }
    }

    bool AssetFile::IsDirty() const
    {
        return currentVersion > persistedVersion;
    }

    void AssetEditor::AddPackage(StringView directory)
    {
        if (AssetFile* assetFile = ScanForAssets(directory))
        {
            packages.EmplaceBack(assetFile);
        }
    }

    AssetFile* AssetEditor::CreateDirectory(AssetFile* parent)
    {
        AssetFile* newDirectory = MemoryGlobals::GetDefaultAllocator().Alloc<AssetFile>();

        newDirectory->fileName = CreateUniqueName(parent, "New Folder");
        newDirectory->absolutePath = Path::Join(parent->absolutePath, newDirectory->fileName);
        newDirectory->hash = HashInt32(HashValue(newDirectory->absolutePath));
        newDirectory->isDirectory = true;
        newDirectory->currentVersion = 1;
        newDirectory->persistedVersion = 0;
        newDirectory->parent = parent;
        assets.Insert(newDirectory->absolutePath, newDirectory);

        parent->children.EmplaceBack(newDirectory);

        return newDirectory;
    }

    AssetFile* AssetEditor::CreateAsset(AssetFile* parent, TypeID typeId)
    {
        return nullptr;
    }

    void AssetEditor::Rename(AssetFile* assetFile, StringView newName)
    {
        assetFile->fileName = newName;
        assetFile->currentVersion++;
    }

    void AssetEditor::GetUpdatedAssets(Array<AssetFile*>& updatedAssets)
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
            if (assetFile->active)
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
            else
            {
                //TODO check asset IO.
            }
        }
    }

    void AssetEditor::DeleteAssets(Span<AssetFile*> assetFiles)
    {
        for (AssetFile* asset : assetFiles)
        {
            asset->active = false;
            asset->currentVersion++;
        }
    }

    String AssetEditor::CreateUniqueName(AssetFile* parent, StringView desiredName)
    {
        if (parent == nullptr) return {};

        u32    count{};
        String finalName = desiredName;
        bool   nameFound;
        do
        {
            nameFound = true;
            for (AssetFile* child : parent->children)
            {
                if (finalName == child->fileName)
                {
                    finalName = desiredName;
                    finalName += " (";
                    finalName.Append(++count);
                    finalName += ")";
                    nameFound = false;
                    break;
                }
            }
        }
        while (!nameFound);
        return finalName;
    }

    void AssetEditor::ImportAssets(Span<String> paths)
    {
        for (const String& path : paths)
        {
            String extension = Path::Extension(path);
            if (auto it = extensionImporters.Find(extension))
            {
                AssetImporter* io = it->second;
                io->ImportAsset(path);
            }
        }
    }

    void AssetEditor::FilterExtensions(Array<FileFilter>& extensions)
    {
        for(auto& it: extensionImporters)
        {
            extensions.EmplaceBack(FileFilter{
                .name = it.first.CStr(),
                .spec = it.first.CStr()
            });
        }
    }

    Span<AssetFile*> AssetEditor::GetPackages()
    {
        return packages;
    }

    void AssetEditorShutdown()
    {
        for(auto& it: assets)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(it.second);
        }

        packages.Clear();
        assets.Clear();

        for(AssetImporter* io : importers)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(io);
        }

        importers.Clear();
        extensionImporters.Clear();
    }

    void AssetEditorInit()
    {
        Event::Bind<OnShutdown, AssetEditorShutdown>();

        importers = Registry::InstantiateDerived<AssetImporter>();
        for(AssetImporter* importer: importers)
        {
            for (const String& extension : importer->ImportExtensions())
            {
                extensionImporters.Insert(extension, importer);
            }
        }
    }
}
