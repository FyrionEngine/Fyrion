#include "AssetEditor.hpp"

#include "AssetTypes.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Core/StaticContent.hpp"

namespace Fyrion
{
    namespace
    {
        Array<AssetFile*>           packages;
        HashMap<String, AssetFile*> assets;

        Array<AssetImporter*>           importers;
        HashMap<String, AssetImporter*> extensionImporters;

        Array<AssetHandler*>           handlers;
        HashMap<String, AssetHandler*> handlersByExtension;
        HashMap<TypeID, AssetHandler*> handlersByTypeID;

        Texture folderTexture = {};
        Texture fileTexture = {};

        Logger& logger = Logger::GetLogger("Fyrion::AssetEditor");

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
                AssetFile* assetFile = AllocateNew(Path::Name(path));
                assetFile->isDirectory = false;
                assetFile->absolutePath = path;
                assetFile->extension = Path::Extension(path);
                assetFile->persistedVersion = 1;

                String infoFile = Path::Join(path, ".info");
                if (FileSystem::GetFileStatus(infoFile).exists)
                {
                    JsonAssetReader jsonAssetReader(FileSystem::ReadFileAsString(infoFile));
                    ArchiveObject   root = jsonAssetReader.ReadObject();

                    assetFile->uuid = UUID::FromString(jsonAssetReader.ReadString(root, "uuid"));
                }
                else
                {
                    assetFile->uuid = UUID::RandomUUID();
                }

                if (auto it = handlersByExtension.Find(assetFile->extension))
                {
                    assetFile->handler = it->second;
                    Assets::Create(assetFile->uuid, assetFile);
                }

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

    Asset* AssetFile::LoadAsset()
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(handler->GetAssetTypeID()))
        {
            Asset* asset = typeHandler->Cast<Asset>(typeHandler->NewInstance());
            asset->SetName(fileName);
            asset->SetTypeHandler(typeHandler);
            handler->Load(this, typeHandler, asset);
            return asset;
        }
        return nullptr;
    }

    Array<u8> AssetFile::LoadStream(usize offset, usize size)
    {
        String bufferFile = Path::Join(absolutePath, ".buffer");

        Array<u8> arr;
        arr.Resize(size);

        FileHandler file = FileSystem::OpenFile(bufferFile, AccessMode::ReadOnly);
        FileSystem::ReadFileAt(file, arr.Data(), size, offset);
        FileSystem::CloseFile(file);

        return arr;
    }

    OutputFileStream AssetFile::CreateStream()
    {
        String bufferFile = Path::Join(absolutePath, ".buffer");
        return OutputFileStream(bufferFile);
    }

    Texture AssetFile::GetThumbnail()
    {
        if (isDirectory)
        {
            return folderTexture;
        }

        if (!thumbnailVerified)
        {
            String cachePath = Path::Join("C:\\dev\\Fyrion\\Temp", uuid.ToString(), ".thumbnail");
            if (FileSystem::GetFileStatus(cachePath).exists)
            {
                Image image(128, 128, 4);
                image.data = FileSystem::ReadFileAsByteArray(cachePath);
                thumbnail = Graphics::CreateTextureFromImage(image);
            }
            else
            {
                if (Image image = handler->GenerateThumbnail(this); !image.Empty())
                {
                    if (FileHandler fileHandler = FileSystem::OpenFile(cachePath, AccessMode::WriteOnly))
                    {
                        FileSystem::WriteFile(fileHandler, image.GetData().Data(), image.GetData().Size());
                        FileSystem::CloseFile(fileHandler);
                    }
                    thumbnail = Graphics::CreateTextureFromImage(image);
                }
            }
            thumbnailVerified = true;
        }

        if (thumbnail)
        {
            return thumbnail;
        }

        return fileTexture;
    }

    AssetFile::~AssetFile()
    {
        if (thumbnail)
        {
            Graphics::DestroyTexture(thumbnail);
        }
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
        FY_ASSERT(parent, "parent cannot be null");

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

    AssetFile* AssetEditor::CreateAsset(AssetFile* parent, TypeID typeId, StringView suggestedName)
    {
        FY_ASSERT(parent, "parent cannot be null");

        if (auto it = handlersByTypeID.Find(typeId))
        {
            TypeHandler* typeHandler = Registry::FindTypeById(typeId);
            String       assetName = suggestedName.Empty() ? String("New ").Append(typeHandler->GetSimpleName()) : String(suggestedName);
            String       absolutePath = Path::Join(parent->absolutePath, assetName, it->second->Extension());

            //TODO find if absolutePath exists.

            AssetFile* newAsset = MemoryGlobals::GetDefaultAllocator().Alloc<AssetFile>();
            newAsset->fileName = CreateUniqueName(parent, assetName);
            newAsset->extension = it->second->Extension();
            newAsset->absolutePath = absolutePath;
            newAsset->hash = HashInt32(HashValue(newAsset->absolutePath));
            newAsset->isDirectory = false;
            newAsset->currentVersion = 1;
            newAsset->persistedVersion = 0;
            newAsset->parent = parent;
            newAsset->uuid = UUID::RandomUUID();
            newAsset->handler = it->second;
            assets.Insert(newAsset->absolutePath, newAsset);

            parent->children.EmplaceBack(newAsset);

            Assets::Create(newAsset->uuid, newAsset);

            return newAsset;
        }
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
                String newAbsolutePath = Path::Join(assetFile->parent->absolutePath, assetFile->fileName, assetFile->extension);
                assets.Erase(assetFile->absolutePath);

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
                else if (auto it = handlersByExtension.Find(assetFile->extension))
                {

                    String infoFile = Path::Join(newAbsolutePath, ".info");
                    JsonAssetWriter writer;
                    ArchiveObject   root = writer.CreateObject();
                    writer.WriteString(root, "uuid", assetFile->uuid.ToString());
                    FileSystem::SaveFileAsString(infoFile, JsonAssetWriter::Stringify(root));

                    AssetHandler* handler = it->second;
                    handler->Save(newAbsolutePath, assetFile);
                }

                assetFile->absolutePath = newAbsolutePath;
                assets.Insert(newAbsolutePath, assetFile);
                assetFile->persistedVersion = assetFile->currentVersion;
            }
            else
            {
                //TODO delete assets
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

    void AssetEditor::ImportAssets(AssetFile* parent, Span<String> paths)
    {
        for (const String& path : paths)
        {
            String extension = Path::Extension(path);
            if (auto it = extensionImporters.Find(extension))
            {
                AssetImporter* io = it->second;
                io->ImportAsset(parent, path);
            }
        }
    }

    void AssetEditor::FilterExtensions(Array<FileFilter>& extensions)
    {
        for (auto& it : extensionImporters)
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
        Graphics::WaitQueue();
        Graphics::DestroyTexture(folderTexture);
        Graphics::DestroyTexture(fileTexture);

        for (auto& it : assets)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(it.second);
        }

        packages.Clear();
        assets.Clear();

        for (AssetImporter* io : importers)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(io);
        }

        importers.Clear();
        extensionImporters.Clear();

        handlers.Clear();
    }

    void AssetEditorInit()
    {
        Event::Bind<OnShutdown, AssetEditorShutdown>();

        importers = Registry::InstantiateDerived<AssetImporter>();

        for (AssetImporter* importer : importers)
        {
            for (const String& extension : importer->ImportExtensions())
            {
                extensionImporters.Insert(extension, importer);
            }
        }

        handlers = Registry::InstantiateDerived<AssetHandler>();

        for (AssetHandler* handler : handlers)
        {
            logger.Debug("registered asset handler for extension {} ", handler->Extension());

            if (StringView extension = handler->Extension(); !extension.Empty())
            {
                handlersByExtension.Insert(extension, handler);
            }

            if (TypeID typeId = handler->GetAssetTypeID(); typeId != 0)
            {
                handlersByTypeID.Insert(typeId, handler);
            }
        }

        folderTexture = StaticContent::GetTextureFile("Content/Images/FolderIcon.png");
        fileTexture = StaticContent::GetTextureFile("Content/Images/file.png");
    }
}
