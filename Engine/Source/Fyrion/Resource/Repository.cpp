//Repository is based on
//https://ruby0x1.github.io/machinery_blog_archive/post/the-story-behind-the-truth-designing-a-data-model/index.html
//https://ruby0x1.github.io/machinery_blog_archive/post/multi-threading-the-truth/index.html

#include <mutex>
#include "Repository.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "ResourceObject.hpp"

#include "concurrentqueue.h"

#define PAGE(value)    u32((value)/FY_REPO_PAGE_SIZE)
#define OFFSET(value)  (u32)((value) & (FY_REPO_PAGE_SIZE - 1))

namespace Fyrion
{
    struct SubObjectSetData
    {
        HashSet<RID> subObjects{};
        HashSet<RID> prototypeRemoved{};
    };

    struct StreamObject
    {
        u64 streamId{};
        FileHandler fileHandler{};
    };

    struct ResourceField
    {
        String            name{};
        usize             index{};
        ResourceFieldType fieldType{};
        TypeHandler*      typeHandler{};
        usize             offset{};
    };

    struct ResourceType
    {
        String name;
        TypeID typeId;
        usize size;
        usize alignment;
        HashMap<String, SharedPtr<ResourceField>> fieldsByName;
        Array<ResourceField*> fieldsByIndex;
        TypeHandler* typeHandler;
    };

    struct ResourceData
    {
        ResourceStorage* storage{};
        VoidPtr memory{};
        Array<VoidPtr> fields{};
        ResourceData* dataOnWrite{};
        bool readOnly = true;
    };

    struct ResourceStorage
    {
        RID rid{};
        UUID uuid{};
        ResourceType* resourceType{};
        std::atomic<ResourceData*> data{};
        ResourceStorage* prototype{};
        ResourceStorage* parent{};
        usize parentIndex = U32_MAX;
        bool markedToDestroy{};
        bool active = true;
        std::atomic<u32> version = 1;
    };

    struct ToDestroyResourceData
    {
        ResourceData*   data{};
        bool            destroySubObjects{};
        bool            destroyResource{};
    };

    struct ResourcePage
    {
        ResourceStorage elements[FY_REPO_PAGE_SIZE];
    };


    namespace
    {
        Allocator& allocator = MemoryGlobals::GetDefaultAllocator();

        Logger& logger = Logger::GetLogger("Fyrion::Repository", LogLevel::Debug);
        HashMap<TypeID, SharedPtr<ResourceType>> resourceTypes{};
        HashMap<String, SharedPtr<ResourceType>> resourceTypesByName{};

        std::atomic_size_t counter{};
        usize              pageCount{};
        ResourcePage*      pages[FY_REPO_PAGE_SIZE]{};
        std::mutex         pageMutex{};

        std::mutex         byUUIDMutex{};
        HashMap<UUID, RID> byUUID{};

        std::mutex           byPathMutex{};
        HashMap<String, RID> byPath{};

        moodycamel::ConcurrentQueue<ToDestroyResourceData> toCollectItems = moodycamel::ConcurrentQueue<ToDestroyResourceData>(100);

        ResourceStorage* GetOrAllocate(RID rid)
        {
            if (pages[rid.page] == nullptr)
            {
                std::unique_lock<std::mutex> lock(pageMutex);
                if (pages[rid.page] == nullptr)
                {
                    pages[rid.page] = static_cast<ResourcePage*>(allocator.MemAlloc(sizeof(ResourcePage), alignof(ResourcePage)));
                    pageCount++;
                }
            }
            return &pages[rid.page]->elements[rid.offset];
        }

        RID GetID()
        {
            u64 index = counter++;
            return RID{OFFSET(index), PAGE(index)};
        }

        RID GetID(UUID uuid)
        {
            if (uuid)
            {
                std::unique_lock<std::mutex> lock(byUUIDMutex);
                if (auto it = byUUID.Find(uuid))
                {
                    return it->second;
                }
            }
            RID rid = GetID();
            {
                std::unique_lock<std::mutex> lock(byUUIDMutex);
                byUUID.Insert(uuid, rid);
            }
            return rid;
        }

        void UpdateVersion(ResourceStorage* resourceStorage)
        {
            resourceStorage->version++;
            if (resourceStorage->parent)
            {
                UpdateVersion(resourceStorage->parent);
            }
        }

        void DestroyData(ResourceData* resourceData, bool destroySubobjects)
        {
//        for (int i = 0; i < m_fields.Size(); ++i)
//        {
//            if (m_fields[i] != nullptr)
//            {
//                m_storage->resourceType->fieldsByIndex[i]->typeHandler->Destructor(m_fields[i]);
//                m_fields[i] = nullptr;
//            }
//        }
//        allocator.MemFree(m_data);


            allocator.DestroyAndFree(resourceData);
        }

        void DestroyStorage(ResourceStorage* resourceStorage)
        {

//            for (auto itEvent: resourceStorage->resourceType->events)
//            {
//                if ((itEvent.Second.EventType & ResourceEventType_Destroy) != 0)
//                {
////				itEvent.Second.Event(itEvent.Second.UserData, ResourceEventType_Destroy, resourceStorage->rid);
//                }
//            }

            //TODO
            if (resourceStorage->data)
            {
//                if (m_storage->resourceType->fieldsByIndex[i]->fieldType == ResourceFieldType::SubObjectSet)
//                {
//                    SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_fields[i]);
//                    if (destroySubObjects)
//                    {
//                        for (auto it: subObjectSetData.SubObjects)
//                        {
//                            DestroyResource(&Context->Pages[it.First.Page]->Elements[it.First.Offset]);
//                        }
//                    }
//                }
//                else if (m_storage->resourceType->fieldsByIndex[i]->fieldType == ResourceFieldType::SubObject)
//                {
//
//                }
                DestroyData(resourceStorage->data, true);
            }



            if (resourceStorage->parent && resourceStorage->parentIndex != U32_MAX && !resourceStorage->parent->markedToDestroy)
            {
                ResourceObject parent = Repository::Write(resourceStorage->parent->rid);
                parent.RemoveFromSubObjectSet(resourceStorage->parentIndex, resourceStorage->rid);
                parent.Commit();
            }

            resourceStorage->~ResourceStorage();
            MemSet(resourceStorage, 0, sizeof(ResourceStorage));
        }
    }


    void Repository::CreateResourceType(const ResourceTypeCreation& resourceTypeCreation)
    {
        SharedPtr<ResourceType> resourceType = MakeShared<ResourceType>();
        resourceType->name        = resourceTypeCreation.name;
        resourceType->typeHandler = nullptr;
        resourceType->typeId      = resourceTypeCreation.typeId;
        resourceType->fieldsByIndex.Resize(resourceTypeCreation.fields.Size());

        for (const ResourceFieldCreation& resourceFieldCreation: resourceTypeCreation.fields)
        {
            FY_ASSERT(resourceFieldCreation.index < resourceTypeCreation.fields.Size(), "The index value cannot be greater than the number of elements");

            auto it = resourceType->fieldsByName.Emplace(resourceFieldCreation.name, MakeShared<ResourceField>(
                resourceFieldCreation.name,
                resourceFieldCreation.index,
                resourceFieldCreation.type)
            ).first;

            FY_ASSERT(!resourceType->fieldsByIndex[resourceFieldCreation.index], "Index duplicated");
            resourceType->fieldsByIndex[resourceFieldCreation.index] = it->second.Get();

            if (resourceFieldCreation.type == ResourceFieldType::Value)
            {
                it->second->typeHandler = Registry::FindTypeById(resourceFieldCreation.valueId);
                FY_ASSERT(it->second->typeHandler, "Type not found");

                it->second->offset = resourceType->size;
                if (it->second->typeHandler)
                {
                    resourceType->size += it->second->typeHandler->GetTypeInfo().size;
                }
            }
            else if (resourceFieldCreation.type == ResourceFieldType::SubObject)
            {
                it->second->offset = resourceType->size;
                it->second->typeHandler = Registry::FindType<RID>();
                resourceType->size += sizeof(RID);
            }
            else if (resourceFieldCreation.type == ResourceFieldType::SubObjectSet)
            {
                it->second->offset = resourceType->size;
                it->second->typeHandler = Registry::FindType<SubObjectSetData>();
                resourceType->size += sizeof(SubObjectSetData);
            }
            else if (resourceFieldCreation.type == ResourceFieldType::Stream)
            {
                it->second->offset = resourceType->size;
                it->second->typeHandler = Registry::FindType<StreamObject>();
                resourceType->size += sizeof(StreamObject);
            }
        }

        resourceTypesByName.Insert(resourceTypeCreation.name, resourceType);
        resourceTypes.Emplace(resourceTypeCreation.typeId, Traits::Move(resourceType));

        logger.Debug("Resource Type {} Created", resourceTypeCreation.name);
    }

    RID Repository::CreateResource(TypeID typeId)
    {
        return CreateResource(typeId, {});
    }

    RID Repository::CreateResource(TypeID typeId, const UUID& uuid)
    {
        RID rid = GetID(uuid);
        ResourceStorage* resourceStorage = GetOrAllocate(rid);
        ResourceType* resourceType = nullptr;

        if (auto it = resourceTypes.Find(typeId))
        {
            resourceType = it->second.Get();
        }

        new(PlaceHolder(), resourceStorage) ResourceStorage{
            .rid = rid,
            .uuid = uuid,
            .resourceType = resourceType,
            .data = {}
        };

        return rid;
    }

    ResourceObject Repository::Read(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return ResourceObject{storage->data};
    }

    ResourceObject Repository::Write(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        ResourceType* resourceType = storage->resourceType;

        ResourceData* data = allocator.Alloc<ResourceData>(storage);
        data->memory = allocator.MemAlloc(resourceType->size, 1);
        data->fields.Resize(resourceType->fieldsByIndex.Size());

        if (storage->data)
        {
            ResourceData* copyData = storage->data.load();
            data->dataOnWrite = copyData;

            for (int i = 0; i < resourceType->fieldsByIndex.Size(); ++i)
            {
                if (copyData->fields[i] != nullptr)
                {
                    data->fields[i] = static_cast<char*>(data->memory) + resourceType->fieldsByIndex[i]->offset;
                    if (resourceType->fieldsByIndex[i]->typeHandler)
                    {
                        resourceType->fieldsByIndex[i]->typeHandler->Copy(copyData->fields[i], data->fields[i]);
                    }
                }
            }
        }
        return ResourceObject{data};
    }

    void Repository::DestroyResource(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        toCollectItems.enqueue(ToDestroyResourceData{
            .data = storage->data,
            .destroySubObjects = true,
            .destroyResource = true
        });
        storage->markedToDestroy = true;
    }

    void Repository::GarbageCollect()
    {
        ToDestroyResourceData data;
        while (toCollectItems.try_dequeue(data))
        {
            if (data.destroyResource)
            {
                DestroyStorage(data.data->storage);
            }
            else
            {
                DestroyData(data.data, data.destroySubObjects);
            }
        }
    }

    TypeID Repository::GetResourceTypeID(const StringView& typeName)
    {
        if (auto it = resourceTypesByName.Find(typeName))
        {
            return it->second->typeId;
        }
        return 0;
    }

    TypeHandler* Repository::GetResourceTypeHandler(ResourceType* resourceType)
    {
        return resourceType->typeHandler;
    }

    StringView Repository::GetResourceTypeName(ResourceType* resourceType)
    {
        return resourceType->name;
    }

    bool Repository::IsInstanced(ResourceType* resourceType)
    {
        //return resourceType->isInstanced;
        return false;
    }

    RID Repository::CreateFromPrototype(RID prototype)
    {
        return CreateFromPrototype(prototype, {});
    }

    RID Repository::CreateFromPrototype(RID prototype, const UUID& uuid)
    {
        RID rid = GetID(uuid);
        ResourceStorage* resourceStorage  = GetOrAllocate(rid);
        ResourceStorage* prototypeStorage = &pages[prototype.page]->elements[prototype.offset];
        FY_ASSERT(prototypeStorage->resourceType, "Prototype can't be created from resources without types");

        ResourceData* data = allocator.Alloc<ResourceData>();
        data->storage = resourceStorage;
        data->memory  = nullptr;
        data->fields.Resize(prototypeStorage->resourceType->fieldsByIndex.Size());

        new(PlaceHolder(), resourceStorage) ResourceStorage{
            .rid = rid,
            .uuid = uuid,
            .resourceType = prototypeStorage->resourceType,
            .data = data,
            .prototype = prototypeStorage
        };
        return rid;
    }

    void Repository::SetUUID(const RID& rid, const UUID& uuid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        storage->uuid = uuid;
        {
            std::unique_lock<std::mutex> lock(byUUIDMutex);
            byUUID.Insert(uuid, rid);
        }
    }

    void Repository::SetPath(const RID& rid, const StringView& path)
    {
        std::unique_lock<std::mutex> lockByPath(byPathMutex);
        byPath.Erase(path);
        byPath.Insert(path, rid);
    }

    void Repository::RemovePath(const StringView& path)
    {
        std::unique_lock<std::mutex> lockByPath(byPathMutex);
        byPath.Erase(path);
    }

    UUID Repository::GetUUID(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->uuid;
    }

    RID Repository::GetPrototypeRID(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        if (storage->prototype)
        {
            return storage->prototype->rid;
        }
        return {};
    }

    RID Repository::GetByUUID(const UUID& uuid)
    {
        std::unique_lock<std::mutex> lock(byUUIDMutex);

        if (auto it = byUUID.Find(uuid))
        {
            return it->second;
        }
        return {};
    }

    RID Repository::GetByPath(const StringView& path)
    {
        std::unique_lock<std::mutex> lock(byPathMutex);

        if (auto it = byPath.Find(path))
        {
            return it->second;
        }
        return {};
    }

    TypeID Repository::GetResourceTypeID(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->resourceType->typeId;
    }

    ResourceType* Repository::GetResourceType(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->resourceType;
    }

    RID Repository::GetOrCreateByUUID(const UUID& uuid)
    {
        return GetOrCreateByUUID(uuid, 0);
    }

    RID Repository::GetOrCreateByUUID(const UUID& uuid, TypeID typeId)
    {
        {
            std::unique_lock<std::mutex> lock(byUUIDMutex);

            auto it = byUUID.Find(uuid);
            if (it != byUUID.end())
            {
                return it->second;
            }
        }

        RID rid = GetID();
        {
            std::unique_lock<std::mutex> lock(byUUIDMutex);
            byUUID.Insert(uuid, rid);
        }

        ResourceStorage* storage      = &pages[rid.page]->elements[rid.offset];
        ResourceType   * resourceType = nullptr;

        if (auto it = resourceTypes.Find(typeId))
        {
            resourceType = it->second.Get();
        }

        new(PlaceHolder(), storage) ResourceStorage{
            .rid = rid,
            .uuid = uuid,
            .resourceType = resourceType,
            .data = {}
        };

        return rid;
    }

    void Repository::ClearValues(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        if (storage->data)
        {
            ResourceData* data = storage->data.load();
            if (data->memory)
            {
                for (int i = 0; i < data->fields.Size(); ++i)
                {
                    if (data->fields[i] != nullptr)
                    {
                        data->storage->resourceType->fieldsByIndex[i]->typeHandler->Destructor(data->fields[i]);
                        data->fields[i] = nullptr;
                    }
                }
                allocator.MemFree(data->memory);
                data->memory = nullptr;
            }
        }
    }

    RID Repository::CloneResource(RID rid)
    {
        FY_ASSERT(false, "Not Implemented yet");
        return RID();
    }

    ConstPtr Repository::Read(RID rid, TypeID typeId)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        if (storage->resourceType->typeId == typeId)
        {
            return storage->data.load()->memory;
        }
        FY_ASSERT(false, "Mapping field is not implemented");
        return nullptr;
    }

    void Repository::InactiveResource(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        storage->active  = false;
        storage->version = 0;
    }

    bool Repository::IsActive(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->active;
    }

    bool Repository::IsAlive(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->rid.id != 0;
    }

    bool Repository::IsEmpty(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return !storage->data || !storage->data.load()->memory;
    }

    u32 Repository::GetVersion(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->version;
    }

    void Repository::Commit(RID rid, ConstPtr pointer)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        if (storage->data)
        {
            toCollectItems.enqueue(ToDestroyResourceData{
                .data = storage->data,
                .destroySubObjects = false,
                .destroyResource = false
            });
        }
        ResourceData* data = allocator.Alloc<ResourceData>();
        data->storage = storage;
        data->memory  = allocator.MemAlloc(storage->resourceType->size, storage->resourceType->alignment);
        storage->resourceType->typeHandler->Copy(pointer, data->memory);
        storage->data.store(data);

        UpdateVersion(storage);
    }

    ///*********************************************************************ResourceObject**************************************************************************************************************


    ResourceObject::ResourceObject(ResourceData* data) : m_data(data)
    {
    }

    void ResourceObject::SetValue(u32 index, ConstPtr pointer)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::Value, "Field is not ResourceFieldType::Value");
        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
        }
        field->typeHandler->Copy(pointer, m_data->fields[index]);
    }

    ConstPtr ResourceObject::GetValue(u32 index)
    {
        ConstPtr ptr = m_data->fields[index];
        if (!ptr && m_data->storage->prototype)
        {
            ResourceObject prototype{m_data->storage->prototype->data};
            return prototype.GetValue(index);
        }
        return ptr;
    }


    void ResourceObject::SetSubObject(u32 index, RID subobject)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObject, "Field is not ResourceFieldType::SubObject");

        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
        }

        ResourceStorage* storage = &pages[subobject.page]->elements[subobject.offset];
        storage->parent      = m_data->storage;
        storage->parentIndex = index;
        new(PlaceHolder(), m_data->fields[index]) RID{subobject};
    }

    RID ResourceObject::GetSubObject(u32 index)
    {
        return GetValue<RID>(index);
    }

    void ResourceObject::AddToSubObjectSet(u32 index, RID subObject)
    {
        AddToSubObjectSet(index, {&subObject, 1});
    }

    void ResourceObject::RemoveFromSubObjectSet(u32 index, RID subObject)
    {

    }

    void ResourceObject::RemoveFromSubObjectSet(u32 index, const Span<RID>& subObjects)
    {

    }

    void ResourceObject::AddToSubObjectSet(u32 index, const Span<RID>& subObjects)
    {

    }

    void ResourceObject::ClearSubObjectSet(u32 index)
    {

    }

    usize ResourceObject::GetSubObjectSetCount(u32 index)
    {
        return 0;
    }

    void ResourceObject::GetSubObjectSet(u32 index, Span<RID> subObjects)
    {

    }

    usize ResourceObject::GetRemoveFromPrototypeSubObjectSetCount(u32 index) const
    {
        return 0;
    }

    void ResourceObject::GetRemoveFromPrototypeSubObjectSet(u32 index, Span<RID> remove) const
    {

    }

    void ResourceObject::RemoveFromPrototypeSubObjectSet(u32 index, const Span<RID>& remove)
    {

    }

    void ResourceObject::RemoveFromPrototypeSubObjectSet(u32 index, RID remove)
    {

    }

    void ResourceObject::CancelRemoveFromPrototypeSubObjectSet(u32 index, const Span<RID>& remove)
    {

    }

    void ResourceObject::CancelRemoveFromPrototypeSubObjectSet(u32 index, RID remove)
    {

    }

    bool ResourceObject::Has(u32 index) const
    {
        return false;
    }

    bool ResourceObject::Has(const StringView& name) const
    {
        return false;
    }

    Array<RID> ResourceObject::GetSubObjectSetAsArray(u32 index)
    {
        return Array<RID>();
    }

    u32 ResourceObject::GetValueCount() const
    {
        return 0;
    }

    u32 ResourceObject::GetIndex(const StringView& name) const
    {
        return 0;
    }

    StringView ResourceObject::GetName(u32 index) const
    {
        return Fyrion::StringView();
    }

    TypeHandler* ResourceObject::GetFieldType(u32 index) const
    {
        return nullptr;
    }

    ResourceFieldType ResourceObject::GetResourceType(u32 index) const
    {
        return ResourceFieldType::SubObject;
    }

    ResourceObject::operator bool() const
    {
        return false;
    }

    void ResourceObject::Commit()
    {
        m_data->readOnly = true;
        if (m_data->dataOnWrite)
        {
            if (m_data->storage->data.compare_exchange_strong(m_data->dataOnWrite, m_data))
            {

            }
        }
        else
        {
            m_data->storage->data = m_data;
            //TODO events
            UpdateVersion(m_data->storage);
        }
    }

    ResourceObject::~ResourceObject()
    {
        if (m_data)
        {
            //DestroyData(m_data);
        }
    }

    void RepositoryInit()
    {
        Repository::CreateResource({});
    }

    void RepositoryShutdown()
    {
        resourceTypes.Clear();
        resourceTypesByName.Clear();
        byUUID.Clear();
        byPath.Clear();
    }

    void RegisterResourceTypes()
    {
        Registry::Type<SubObjectSetData>();
        Registry::Type<StreamObject>();
    }
}
