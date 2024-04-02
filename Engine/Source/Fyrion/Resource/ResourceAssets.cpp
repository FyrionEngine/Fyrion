
#include "ResourceAssets.hpp"
#include "Repository.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "ResourceSerialization.hpp"

namespace Fyrion::ResourceAssets
{
    void LoadAssetFile(ResourceObject& assetRoot, RID directoryAsset, const StringView& assetFile);
    String MakeDirectoryAbsolutePath(RID rid);
    String MakeAssetAbsolutePath(RID rid);
    void UpdateStreams(RID object, const StringView& assetFile);
}

namespace Fyrion
{
    struct AssetFileInfo
    {
        u32    LoadedVersion;
        String AbsolutePath;
    };

    struct RepositoryAssetsContext
    {
        HashMap<String, FnImportAsset> AssetImporters{};
        HashMap<String, RID> AssetRoots{};
        HashMap<RID, AssetFileInfo> AssetFileInfos{};
        Logger& Logger = Logger::GetLogger("Fyrion::ResourceAssets", LogLevel::Debug);
    };

    namespace
    {
        RepositoryAssetsContext* Context = nullptr;
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

            Context->AssetFileInfos.Insert(rid, AssetFileInfo{
                .LoadedVersion = Repository::GetVersion(rid),
                .AbsolutePath = assetFile
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

//            FileSystem::LoadTextFile(assetFile, buffer);

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

            Context->AssetFileInfos.Insert(rid, AssetFileInfo{
                .LoadedVersion = Repository::GetVersion(rid),
                .AbsolutePath = assetFile
            });
        }
        else if (auto it = Context->AssetImporters.Find(extension))
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

                Context->AssetFileInfos.Insert(rid, AssetFileInfo{
                    .LoadedVersion = Repository::GetVersion(rid),
                    .AbsolutePath = assetFile
                });
            }
        }
    }

    void ResourceAssets::UpdateStreams(RID object, const StringView& assetFile)
    {
//        Path dataPath = assetFile.Parent() + assetFile.Name() + FY_DATA_EXTENSION;
//        if (FileSystem::GetFileStatus(dataPath).exists)
//        {
//            ResourceObject objectNoPro = Repository::ReadNoPrototypes(object);
//            u32 valueCount = objectNoPro.GetValueCount();
//            for (int i = 0; i < valueCount; ++i)
//            {
//                if (objectNoPro.GetResourceType(i) == ResourceFieldType_Buffer)
//                {
//                    BufferObject* bufferObject = objectNoPro.GetBuffer(i);
//                    if (bufferObject)
//                    {
//                        char strBuffer[17]{};
//                        usize bufSize = U64ToHex(Buffer::GetBufferId(bufferObject), strBuffer);
//                        StringView bufferName = {strBuffer, bufSize};
//                        Path bufferPath =  dataPath + bufferName;
//                        Buffer::MapTo(bufferObject, bufferPath, 0);
//                    }
//                }
//            }
//        }
    }

    RID ResourceAssets::LoadAssetsFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return {};
        }
        RID rid = Repository::CreateResource<AssetRoot>();
        Context->AssetRoots.Insert(name, rid);

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

        Context->AssetFileInfos.Insert(rid, AssetFileInfo{
            .LoadedVersion = Repository::GetVersion(rid),
            .AbsolutePath = directory
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
        Context->Logger.Debug("Saving {} to {} ", name, directory);

        Array<RID> directories = assetRoot.GetSubObjectSetAsArray(AssetRoot::Directories);

        for (RID dir: directories)
        {
            auto it = Context->AssetFileInfos.Find(dir);
            if (it == Context->AssetFileInfos.end())
            {
                it = Context->AssetFileInfos.Insert(dir, AssetFileInfo{}).first;
            }
            AssetFileInfo& info = it->second;
            u32 version = Repository::GetVersion(dir);

            if (version > info.LoadedVersion)
            {
                bool   oldPathExists   = FileSystem::GetFileStatus(info.AbsolutePath).exists;
                String newAbsolutePath = MakeDirectoryAbsolutePath(dir);

                if (oldPathExists)
                {
                    FileSystem::Rename(info.AbsolutePath, newAbsolutePath);
                    Context->Logger.Debug("Directory {} moved from {} to {} ", rid.id, info.AbsolutePath, newAbsolutePath);
                }
                else if (!FileSystem::GetFileStatus(newAbsolutePath).exists)
                {
                    FileSystem::CreateDirectory(newAbsolutePath);
                    Context->Logger.Debug("Directory {} created on {} ", rid.id, newAbsolutePath);
                }

                info.AbsolutePath  = newAbsolutePath;
                info.LoadedVersion = version;
            }
            else if (!Repository::IsActive(dir))
            {
                if (FileSystem::GetFileStatus(info.AbsolutePath).exists)
                {
                    FileSystem::Remove(info.AbsolutePath);
                    Context->Logger.Debug("Directory {} deleted from {} ", rid.id, info.AbsolutePath);
                }
                Repository::DestroyResource(dir);
                Context->AssetFileInfos.Erase(it);
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

            auto it = Context->AssetFileInfos.Find(asset);
            if (it == Context->AssetFileInfos.end())
            {
                it = Context->AssetFileInfos.Insert(asset, AssetFileInfo{}).first;
            }
            AssetFileInfo& info = it->second;
            u32 version = Repository::GetVersion(asset);

            if (version > info.LoadedVersion)
            {
                String newAbsolutePath = MakeAssetAbsolutePath(asset);
                if (!newAbsolutePath.Empty())
                {
                    bool oldPathExists = FileSystem::GetFileStatus(info.AbsolutePath).exists;
                    if (oldPathExists)
                    {
                        FileSystem::Remove(info.AbsolutePath);
                        Context->Logger.Debug("Asset {} Removed from {} ", rid.id, info.AbsolutePath);
                    }

                    String parentPath = Path::Parent(newAbsolutePath);
                    if (!FileSystem::GetFileStatus(parentPath).exists)
                    {
                        FileSystem::CreateDirectory(parentPath);
                    }

                    String dataPath = Path::Join(parentPath, Path::Name(newAbsolutePath), FY_DATA_EXTENSION);

                    String str = ResourceSerialization::WriteResourceInfo(objectRid);
                    //FileSystem::SaveFile(newAbsolutePath, {reinterpret_cast<u8*>(str.begin()), reinterpret_cast<u8*>(str.end())});

                    Context->Logger.Debug("Asset {} saved on {} ", rid.id, newAbsolutePath);

                    ResourceObject object = Repository::Read(objectRid);
                    u32 valueCount = object.GetValueCount();
                    for (int i = 0; i < valueCount; ++i)
                    {
                        if (object.HasNoPrototype(i))
                        {
                            if (object.GetResourceType(i) == ResourceFieldType::Stream)
                            {
//                            BufferObject* bufferObject = object.GetBuffer(i);
//                            if (bufferObject)
//                            {
//                                char strBuffer[17]{};
//                                usize bufSize = U64ToHex(Buffer::GetBufferId(bufferObject), strBuffer);
//                                StringView bufferName = {strBuffer, bufSize};
//                                Path bufferPath = dataPath + bufferName;
//
//
//                                if (!FileSystem::GetFileStatus(dataPath).exists)
//                                {
//                                    FileSystem::CreateDirectory(dataPath);
//                                }
//
//                                //if it's still mapped, it's not changed.
//                                StringView path = Buffer::MappedTo(bufferObject);
//                                if (!path.Empty())
//                                {
//                                    if (bufferPath.GetString() != path)
//                                    {
//                                        FileSystem::Rename(path, bufferPath);
//                                    }
//                                }
//                                else
//                                {
//                                    Path tempPath = FileSystem::TempFolder() + bufferName;
//                                    FileSystem::Rename(tempPath, bufferPath);
//                                }
//                                Buffer::MapTo(bufferObject, bufferPath, 0);
//                            }
                            }
                        }
                    }

                    String oldPath = info.AbsolutePath;
                    String oldDataPath = Path::Join(Path::Parent(info.AbsolutePath), Path::Name(info.AbsolutePath), FY_DATA_EXTENSION);
                    if (oldDataPath != dataPath)
                    {
                        FileSystem::Remove(oldDataPath);
                    }

                    info.AbsolutePath = newAbsolutePath;
                }
                info.LoadedVersion   = version;
            }
            else if (!Repository::IsActive(asset))
            {
                if (FileSystem::GetFileStatus(info.AbsolutePath).exists)
                {
                    FileSystem::Remove(info.AbsolutePath);
                    Context->Logger.Debug("Asset {} deleted from {} ", rid.id, info.AbsolutePath);
                }

                String dataPath = Path::Join(Path::Parent(info.AbsolutePath), Path::Name(info.AbsolutePath), FY_DATA_EXTENSION);
                if (FileSystem::GetFileStatus(dataPath).exists)
                {
                    FileSystem::Remove(dataPath);
                    Context->Logger.Debug("Asset Data Directory {} deleted from {} ", rid.id, dataPath);
                }

                Repository::DestroyResource(asset);
                Context->AssetFileInfos.Erase(it);
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
        if (auto it = Context->AssetRoots.Find(name))
        {
            return it->second;
        }
        return {};
    }

    u32 ResourceAssets::GetLoadedVersion(RID rid)
    {
        if (auto it = Context->AssetFileInfos.Find(rid))
        {
            return it->second.LoadedVersion;
        }
        return 0;
    }

    StringView ResourceAssets::GetAbsolutePath(RID rid)
    {
        if (auto it = Context->AssetFileInfos.Find(rid))
        {
            return it->second.AbsolutePath;
        }
        return {};
    }

    void ResourceAssets::ImportAsset(RID root, RID directory, const StringView& absolutePath)
    {
        String extension = Path::Extension(absolutePath);

        if (auto it = Context->AssetImporters.Find(extension))
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
            Context->AssetImporters.Insert(extension, fnImportAsset);
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

//    void DirectoryChanges(CPtr userData, ResourceEventType eventType, ResourceObject& resourceObject)
//    {
//        const ResourceData* writeData = resourceObject.GetWriteData();
//        RID parent = *static_cast<const RID*>(Repository::GetValue(writeData, AssetDirectory::Parent));
//        const String& name = *static_cast<const String*>(Repository::GetValue(writeData, AssetDirectory::Name));
//
//        String path = ResourceAssets::GetPath(parent) + "/" + name;
//        resourceObject.SetValue(AssetDirectory::Path, path);
//        Repository::SetPath(resourceObject.GetRID(), path);
//    }
//
//    void AssetChanges(CPtr userData, ResourceEventType eventType, ResourceObject& resourceObject)
//    {
//        const ResourceData* writeData = resourceObject.GetWriteData();
//        RID parent = *static_cast<const RID*>(Repository::GetValue(writeData, Asset::Directory));
//        const String& name = *static_cast<const String*>(Repository::GetValue(writeData, Asset::Name));
//        const String& extension = *static_cast<const String*>(Repository::GetValue(writeData, Asset::Extension));
//        String path = ResourceAssets::GetPath(parent) + "/" + name + extension;
//        resourceObject.SetValue(Asset::Path, path);
//
//        const RID* object = static_cast<const RID*>(Repository::GetValue(writeData, Asset::Object));
//        if (object)
//        {
//            Repository::SetPath(*object, path);
//            Context->Logger.Debug("Asset Registered to Path {} ", path);
//        }
//    }

    void RepositoryAssetsInit()
    {
//        Context = Alloc<RepositoryAssetsContext>();
//
//        //asset root
//        {
//            FixedArray<ResourceFieldCreation, 4> fields{
//                ResourceFieldCreation{.Index = AssetRoot::Name, .Name = "Name", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()},
//                ResourceFieldCreation{.Index = AssetRoot::Assets, .Name = "Assets", .Type =  ResourceFieldType_SubObjectSet},
//                ResourceFieldCreation{.Index = AssetRoot::Directories, .Name = "Directories", .Type =  ResourceFieldType_SubObjectSet},
//                ResourceFieldCreation{.Index = AssetRoot::Path, .Name = "Path", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()},
//            };
//            ResourceTypeCreation resourceTypeCreation{.Name = "AssetRoot", .TypeId = AssetRoot, .Fields = fields};
//            Repository::CreateResourceType(resourceTypeCreation);
//        }
//
//        //asset directory
//        {
//            FixedArray<ResourceFieldCreation, 3> fields{
//                ResourceFieldCreation{.Index = AssetDirectory::Name, .Name = "Name", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()},
//                ResourceFieldCreation{.Index = AssetDirectory::Parent, .Name = "Parent", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<RID>()},
//                ResourceFieldCreation{.Index = AssetDirectory::Path, .Name = "Path", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()}
//            };
//            ResourceTypeCreation resourceTypeCreation{.Name = "AssetDirectory", .TypeId = AssetDirectory, .Fields = fields};
//            Repository::CreateResourceType(resourceTypeCreation);
//            Repository::AddResourceTypeEvent(AssetDirectory, nullptr, ResourceEventType_Insert | ResourceEventType_Update, DirectoryChanges);
//        }
//
//        //assets
//        {
//            FixedArray<ResourceFieldCreation, 5> fields{
//                ResourceFieldCreation{.Index = Asset::Name, .Name = "Name", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()},
//                ResourceFieldCreation{.Index = Asset::Directory, .Name = "Directory", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<RID>()},
//                ResourceFieldCreation{.Index = Asset::Object, .Name = "Object", .Type =  ResourceFieldType_SubObject},
//                ResourceFieldCreation{.Index = Asset::Path, .Name = "Path", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()},
//                ResourceFieldCreation{.Index = Asset::Extension, .Name = "Extension", .Type =  ResourceFieldType_Value, .FieldTypeId = GetTypeID<String>()}
//            };
//            ResourceTypeCreation resourceTypeCreation{.Name = "Asset", .TypeId = Asset, .Fields = fields};
//            Repository::CreateResourceType(resourceTypeCreation);
//            Repository::AddResourceTypeEvent(Asset, nullptr, ResourceEventType_Insert | ResourceEventType_Update, AssetChanges);
//        }
    }

    void RepositoryAssetsShutdown()
    {
//        DestroyAndFree(Context);
    }
}