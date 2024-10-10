#include "AssetEditor.hpp"

#include "AssetImporter.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    bool AssetFile::IsDirty() const
    {
        return currentVersion > persistedVersion;
    }

    void AssetEditor::Init()
    {
        TypeHandler*      assetImporterType = Registry::FindType<AssetImporter>();
        Span<DerivedType> derivedTypes = assetImporterType->GetDerivedTypes();
        for (const DerivedType& derivedType : derivedTypes)
        {
            if (TypeHandler* type = Registry::FindTypeById(derivedType.typeId))
            {
                AssetImporter* assetImporter = type->Cast<AssetImporter>(type->NewInstance());
                for (const String& extension : assetImporter->ImportExtensions())
                {
                    extensionImporters.Insert(extension, assetImporter);
                }
                importers.EmplaceBack(assetImporter);
            }
        }
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
                AssetImporter* importer = it->second;


                importer->ImportAsset(*this, path, nullptr);
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
            childFile->parent = assetFile;
            assetFile->children.EmplaceBack(childFile);
            UpdateCache(child, childFile);
        }
    }
}
