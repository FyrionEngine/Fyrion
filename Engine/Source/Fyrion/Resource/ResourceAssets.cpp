
#include "ResourceAssets.hpp"
#include "Repository.hpp"
#include "Fyrion/Core/logger.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "ResourceSerialization.hpp"

namespace Fyrion::ResourceAssets
{
    void LoadAssetFile(ResourceObject& assetRoot, RID directoryAsset, const StringView& assetFile);
    String MakeDirectoryAbsolutePath(RID rid);
    String MakeAssetAbsolutePath(RID rid);
    void UpdateStreams(RID rid, const StringView& assetFile);
}

namespace Fyrion
{
    struct AssetFileInfo
    {
        u32    loadedVersion;
        String absolutePath;
    };

    namespace
    {
        HashMap<String, FnImportAsset>      assetImporters{};
        HashMap<String, RID>                assetRoots{};
        HashMap<RID, AssetFileInfo>         assetFileInfos{};
        Logger& logger = Logger::GetLogger("Fyrion::ResourceAssets", LogLevel::Debug);
    }

    void ResourceAssets::LoadAssetFile(ResourceObject& assetRoot, RID directoryAsset, const StringView& assetFile)
    {
        String extension = Path::Extension(assetFile);
        if (extension == FY_DATA_EXTENSION) return;

        if (FileSystem::GetFileStatus(assetFile).isDirectory)
        {
            RID rid = Repository::CreateResource<AssetDirectory>();

            ResourceObject assetDirectory = Repository::Write(rid);
            assetDirectory.SetValue(AssetDirectory::Name, Path::Name(assetFile));
            assetDirectory.SetValue(AssetDirectory::Parent, directoryAsset);
            assetDirectory.Commit();

            assetFileInfos.Insert(rid, AssetFileInfo{
                .loadedVersion = Repository::GetVersion(rid),
                .absolutePath = assetFile
            });

            assetRoot.AddToSubObjectSet(AssetRoot::Directories, rid);

            for (const auto& entry: DirectoryEntries{assetFile})
            {
                LoadAssetFile(assetRoot, rid, entry);
            }
        }
        else if (extension == FY_ASSET_EXTENSION)
        {
            String buffer = {};

            FileHandler handler = FileSystem::OpenFile(assetFile, AccessMode::ReadOnly);
            usize size = FileSystem::GetFileSize(handler);
            buffer.Resize(size);
            FileSystem::ReadFile(handler, buffer.begin(), size);
            FileSystem::CloseFile(handler);

            if (buffer.Empty()) return;

            RID object = ResourceSerialization::ParseResourceInfo(buffer);

            UpdateStreams(object, assetFile);

            RID rid = Repository::CreateResource<Asset>();
            ResourceObject asset = Repository::Write(rid);
            asset.SetValue(Asset::Name, Path::Name(assetFile));
            asset.SetValue(Asset::Directory, directoryAsset);
            asset.SetSubObject(Asset::Object, object);
            asset.SetValue(Asset::Extension, FY_ASSET_EXTENSION);
            asset.Commit();

            assetRoot.AddToSubObjectSet(AssetRoot::Assets, rid);

            assetFileInfos.Insert(rid, AssetFileInfo{
                .loadedVersion = Repository::GetVersion(rid),
                .absolutePath = assetFile
            });
        }
        else if (auto it = assetImporters.Find(extension))
        {
            FnImportAsset importAsset = it->second;
            if (importAsset)
            {
                RID rid = Repository::CreateResource<Asset>();

                ResourceObject asset = Repository::Write(rid);
                asset.SetValue(Asset::Name, Path::Name(assetFile));
                asset.SetValue(Asset::Directory, directoryAsset);
                asset.SetValue(Asset::Extension, Path::Extension(assetFile));

                assetRoot.AddToSubObjectSet(AssetRoot::Assets, rid);

                RID object = importAsset(rid, assetFile);
                if (object)
                {
                    asset.SetSubObject(Asset::Object, object);
                }

                asset.Commit();

                assetFileInfos.Insert(rid, AssetFileInfo{
                    .loadedVersion = Repository::GetVersion(rid),
                    .absolutePath = assetFile
                });
            }
        }
    }

    void ResourceAssets::UpdateStreams(RID rid, const StringView& assetFile)
    {
        String dataPath =  Path::Join(Path::Parent(assetFile), Path::Name(assetFile), FY_DATA_EXTENSION);
        if (FileSystem::GetFileStatus(dataPath).exists)
        {
            ResourceObject read = Repository::Read(rid);
            u32 valueCount = read.GetValueCount();
            for (int i = 0; i < valueCount; ++i)
            {
                if (read.HasNoPrototype(i) && read.GetResourceType(i) == ResourceFieldType::SubObjectSet)
                {
                    StreamObject* streamObject = read.GetStream(i);
                    if (streamObject)
                    {
                        char strBuffer[17]{};
                        usize bufSize = U64ToHex(streamObject->GetBufferId(), strBuffer);
                        StringView streamName = {strBuffer, bufSize};
                        String streamPath =  Path::Join(dataPath, streamName);
                        streamObject->MapTo(streamPath, 0);
                    }
                }
            }
        }
    }

    RID ResourceAssets::LoadAssetsFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return {};
        }
        RID rid = Repository::CreateResource<AssetRoot>();
        assetRoots.Insert(name, rid);

        String path = String{name} + ":/";
        Repository::SetPath(rid, path);
        {
            ResourceObject assetRoot = Repository::Write(rid);
            assetRoot.SetValue(AssetRoot::Name, name);
            assetRoot.SetValue(AssetRoot::Path, path);
            assetRoot.Commit();
        }

        {
            ResourceObject assetRoot = Repository::Write(rid);
            for (const auto& entry: DirectoryEntries{directory})
            {
                LoadAssetFile(assetRoot, rid, entry);
            }
            assetRoot.Commit();
        }

        assetFileInfos.Insert(rid, AssetFileInfo{
            .loadedVersion = Repository::GetVersion(rid),
            .absolutePath = directory
        });

        return rid;
    }

    void ResourceAssets::SaveAssetsToDirectory(RID rid, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            FileSystem::CreateDirectory(directory);
        }

        ResourceObject assetRoot = Repository::Read(rid);
        const String& name = assetRoot.GetValue<String>(AssetRoot::Name);
        logger.Debug("Saving {} to {} ", name, directory);

        Array<RID> directories = assetRoot.GetSubObjectSetAsArray(AssetRoot::Directories);

        for (RID dir: directories)
        {
            auto it = assetFileInfos.Find(dir);
            if (it == assetFileInfos.end())
            {
                it = assetFileInfos.Insert(dir, AssetFileInfo{}).first;
            }
            AssetFileInfo& info = it->second;
            u32 version = Repository::GetVersion(dir);

            if (version > info.loadedVersion)
            {
                bool   oldPathExists   = FileSystem::GetFileStatus(info.absolutePath).exists;
                String newAbsolutePath = MakeDirectoryAbsolutePath(dir);

                if (oldPathExists)
                {
                    FileSystem::Rename(info.absolutePath, newAbsolutePath);
                    logger.Debug("Directory {} moved from {} to {} ", rid.id, info.absolutePath, newAbsolutePath);
                }
                else if (!FileSystem::GetFileStatus(newAbsolutePath).exists)
                {
                    FileSystem::CreateDirectory(newAbsolutePath);
                    logger.Debug("Directory {} created on {} ", rid.id, newAbsolutePath);
                }

                info.absolutePath  = newAbsolutePath;
                info.loadedVersion = version;
            }
            else if (!Repository::IsActive(dir))
            {
                if (FileSystem::GetFileStatus(info.absolutePath).exists)
                {
                    FileSystem::Remove(info.absolutePath);
                    logger.Debug("Directory {} deleted from {} ", rid.id, info.absolutePath);
                }
                Repository::DestroyResource(dir);
                assetFileInfos.Erase(it);
            }
        }

        Array<RID> assets = assetRoot.GetSubObjectSetAsArray(AssetRoot::Assets);
        for (RID asset: assets)
        {
            ResourceObject assetObject = Repository::Read(asset);
            RID objectRid = assetObject[Asset::Object].As<RID>();
            if (!Repository::GetUUID(objectRid))
            {
                continue;
            }

            auto it = assetFileInfos.Find(asset);
            if (it == assetFileInfos.end())
            {
                it = assetFileInfos.Insert(asset, AssetFileInfo{}).first;
            }
            AssetFileInfo& info = it->second;
            u32 version = Repository::GetVersion(asset);

            if (version > info.loadedVersion)
            {
                String newAbsolutePath = MakeAssetAbsolutePath(asset);
                if (!newAbsolutePath.Empty())
                {
                    bool oldPathExists = FileSystem::GetFileStatus(info.absolutePath).exists;
                    if (oldPathExists)
                    {
                        FileSystem::Remove(info.absolutePath);
                        logger.Debug("Asset {} Removed from {} ", rid.id, info.absolutePath);
                    }

                    String parentPath = Path::Parent(newAbsolutePath);
                    if (!FileSystem::GetFileStatus(parentPath).exists)
                    {
                        FileSystem::CreateDirectory(parentPath);
                    }

                    String dataPath = Path::Join(parentPath, Path::Name(newAbsolutePath), FY_DATA_EXTENSION);

                    String str = ResourceSerialization::WriteResourceInfo(objectRid);

                    FileHandler handler = FileSystem::OpenFile(newAbsolutePath, AccessMode::WriteOnly);
                    FileSystem::WriteFile(handler, str.begin(), str.Size());
                    FileSystem::CloseFile(handler);

                    logger.Debug("Asset {} saved on {} ", rid.id, newAbsolutePath);

                    ResourceObject object = Repository::Read(objectRid);
                    u32 valueCount = object.GetValueCount();
                    for (int i = 0; i < valueCount; ++i)
                    {
                        if (object.HasNoPrototype(i))
                        {
                            if (object.GetResourceType(i) == ResourceFieldType::Stream)
                            {
                            StreamObject* streamObject = object.GetStream(i);
                            if (streamObject)
                            {
                                char strBuffer[17]{};
                                usize bufSize = U64ToHex(streamObject->GetBufferId(), strBuffer);
                                StringView streamName = {strBuffer, bufSize};
                                String streamPath = Path::Join(dataPath, streamName);

                                if (!FileSystem::GetFileStatus(dataPath).exists)
                                {
                                    FileSystem::CreateDirectory(dataPath);
                                }

                                //if it's still mapped, it's not changed.
                                StringView path = streamObject->MappedTo();
                                if (!path.Empty())
                                {
                                    if (streamPath != path)
                                    {
                                        FileSystem::Rename(path, streamPath);
                                    }
                                }
                                else
                                {
                                    String tempPath = Path::Join(FileSystem::TempFolder(), streamName);
                                    FileSystem::Rename(tempPath, streamPath);
                                }
                                streamObject->MapTo(streamPath, 0);
                            }
                            }
                        }
                    }

                    String oldPath = info.absolutePath;
                    String oldDataPath = Path::Join(Path::Parent(info.absolutePath), Path::Name(info.absolutePath), FY_DATA_EXTENSION);
                    if (oldDataPath != dataPath)
                    {
                        FileSystem::Remove(oldDataPath);
                    }

                    info.absolutePath = newAbsolutePath;
                }
                info.loadedVersion   = version;
            }
            else if (!Repository::IsActive(asset))
            {
                if (FileSystem::GetFileStatus(info.absolutePath).exists)
                {
                    FileSystem::Remove(info.absolutePath);
                    logger.Debug("Asset {} deleted from {} ", rid.id, info.absolutePath);
                }

                String dataPath = Path::Join(Path::Parent(info.absolutePath), Path::Name(info.absolutePath), FY_DATA_EXTENSION);
                if (FileSystem::GetFileStatus(dataPath).exists)
                {
                    FileSystem::Remove(dataPath);
                    logger.Debug("Asset Data Directory {} deleted from {} ", rid.id, dataPath);
                }

                Repository::DestroyResource(asset);
                assetFileInfos.Erase(it);
            }
        }
    }

    String ResourceAssets::GetName(RID asset)
    {
        TypeID typeId = Repository::GetResourceTypeID(asset);

        ResourceObject resourceObject = Repository::Read(asset);
        if (typeId == GetTypeID<AssetDirectory>())
        {
            return resourceObject[AssetDirectory::Name].As<String>();
        }
        else if (typeId == GetTypeID<Asset>())
        {
            return resourceObject[Asset::Name].As<String>();
        }
        else if (typeId == GetTypeID<AssetRoot>())
        {
            return resourceObject[AssetRoot::Name].As<String>();
        }
        return "";
    }

    RID ResourceAssets::GetParent(RID asset)
    {
        TypeID typeId = Repository::GetResourceTypeID(asset);
        ResourceObject resourceObject = Repository::Read(asset);
        if (typeId == GetTypeID<AssetDirectory>())
        {
            return resourceObject[AssetDirectory::Parent].As<RID>();
        }
        else if (typeId == GetTypeID<Asset>())
        {
            return resourceObject[Asset::Directory].As<RID>();
        }
        return {};
    }

    RID ResourceAssets::GetAssetRootByName(const StringView& name)
    {
        if (auto it = assetRoots.Find(name))
        {
            return it->second;
        }
        return {};
    }

    u32 ResourceAssets::GetLoadedVersion(RID rid)
    {
        if (auto it = assetFileInfos.Find(rid))
        {
            return it->second.loadedVersion;
        }
        return 0;
    }

    StringView ResourceAssets::GetAbsolutePath(RID rid)
    {
        if (auto it = assetFileInfos.Find(rid))
        {
            return it->second.absolutePath;
        }
        return {};
    }

    void ResourceAssets::ImportAsset(RID root, RID directory, const StringView& absolutePath)
    {
        String extension = Path::Extension(absolutePath);

        if (auto it = assetImporters.Find(extension))
        {
            FnImportAsset importAsset = it->second;
            if (importAsset)
            {
                ResourceObject assetRoot = Repository::Write(root);
                RID rid = Repository::CreateResource<Asset>();

                ResourceObject asset = Repository::Write(rid);
                asset.SetValue(Asset::Name, Path::Name(absolutePath));
                asset.SetValue(Asset::Directory, directory);
                asset.SetValue(Asset::Extension, FY_ASSET_EXTENSION);

                assetRoot.AddToSubObjectSet(AssetRoot::Assets, rid);

                RID object = importAsset(rid, absolutePath);
                if (object)
                {
                    asset.SetSubObject(Asset::Object, object);
                    if (!Repository::GetUUID(object))
                    {
                        Repository::SetUUID(object, UUID::RandomUUID());
                    }
                }
                asset.Commit();
                assetRoot.Commit();
            }
        }
    }

    String ResourceAssets::GetPath(RID rid)
    {
        if (Repository::GetResourceTypeID(rid) == GetTypeID<Asset>())
        {
            ResourceObject obj = Repository::Read(rid);
            return obj[Asset::Path].As<String>();
        }
        else if (Repository::GetResourceTypeID(rid) == GetTypeID<AssetDirectory>())
        {
            ResourceObject obj = Repository::Read(rid);
            return obj[AssetDirectory::Path].As<String>();
        }
        else if (Repository::GetResourceTypeID(rid) == GetTypeID<AssetRoot>())
        {
            ResourceObject obj = Repository::Read(rid);
            return obj[AssetRoot::Path].As<String>();
        }
        return {};
    }

    void ResourceAssets::AddAssetImporter(StringView extensions, FnImportAsset fnImportAsset)
    {
        Split(extensions, StringView{","}, [&](const StringView& extension)
        {
            assetImporters.Insert(extension, fnImportAsset);
        });
    }

    String ResourceAssets::MakeDirectoryAbsolutePath(RID rid)
    {
        if (!rid) return {};

        ResourceObject resourceObject = Repository::Read(rid);

        RID parent = resourceObject[AssetDirectory::Parent].As<RID>();

        String parentPath = GetAbsolutePath(parent);
        if (parentPath.Empty())
        {
            parentPath = MakeDirectoryAbsolutePath(parent);
        }
        const String& assetName = resourceObject.GetValue<String>(AssetDirectory::Name);
        return Path::Join(parentPath, assetName);
    }

    String ResourceAssets::MakeAssetAbsolutePath(RID rid)
    {
        if (!rid) return {};

        ResourceObject resourceObject = Repository::Read(rid);

        RID parent = resourceObject[Asset::Directory].As<RID>();

        String parentPath = GetAbsolutePath(parent);
        if (parentPath.Empty())
        {
            parentPath = MakeDirectoryAbsolutePath(parent);
        }
        const String& assetName = resourceObject.GetValue<String>(Asset::Name);

        return Path::Join(parentPath, assetName, FY_ASSET_EXTENSION);
    }

    void DirectoryChanges(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldData, ResourceObject& newData)
    {
        RID parent = newData[AssetDirectory::Parent].As<RID>();
        const String& name =  newData[AssetDirectory::Name].As<String>();

        String path = ResourceAssets::GetPath(parent) + "/" + name;
        newData.SetValue(AssetDirectory::Path, path);
        Repository::SetPath(newData.GetRID(), path);
    }
//
    void AssetChanges(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldData, ResourceObject& newData)
    {
        RID parent = newData[Asset::Directory].As<RID>();
        const String& name = newData[Asset::Name].As<String>();
        const String& extension = newData[Asset::Extension].As<String>();

        String path = ResourceAssets::GetPath(parent) + "/" + name + extension;
        newData.SetValue(Asset::Path, path);

        RID* object = (RID*) newData.GetValue(Asset::Object);
        if (object)
        {
            Repository::SetPath(*object, path);
            logger.Debug("Asset Registered to Path {} ", path);
        }
    }

    void ResourceAssetsInit()
    {
        ResourceTypeBuilder<AssetRoot>::Builder()
            .Value<AssetRoot::Name, String>("Name")
            .SubObjectSet<AssetRoot::Assets>("Assets")
            .SubObjectSet<AssetRoot::Directories>("Directories")
            .Value<AssetRoot::Path, String>("Path")
            .Build();

        ResourceTypeBuilder<AssetDirectory>::Builder()
            .Value<AssetDirectory::Name, String>("Name")
            .Value<AssetDirectory::Parent, String>("RID")
            .Value<AssetDirectory::Path, String>("Path")
            .Build();

        Repository::AddResourceTypeEvent(GetTypeID<AssetDirectory>(), nullptr, ResourceEventType::Insert | ResourceEventType::Update, DirectoryChanges);

        ResourceTypeBuilder<Asset>::Builder()
            .Value<Asset::Name, String>("Name")
            .Value<Asset::Directory, RID>("Directory")
            .SubObject<Asset::Object>("Object")
            .Value<Asset::Path, String>("Path")
            .Value<Asset::Extension, String>("Extension")
            .Build();

        Repository::AddResourceTypeEvent(GetTypeID<Asset>(), nullptr, ResourceEventType::Insert | ResourceEventType::Update, AssetChanges);
    }

    void ResourceAssetsShutdown()
    {
        assetImporters.Clear();
        assetRoots.Clear();
        assetFileInfos.Clear();
    }
}