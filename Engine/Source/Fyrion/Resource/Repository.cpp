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
#include "StreamObject.hpp"

#include "concurrentqueue.h"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

#define PAGE(value)    u32((value)/FY_REPO_PAGE_SIZE)
#define OFFSET(value)  (u32)((value) & (FY_REPO_PAGE_SIZE - 1))

namespace Fyrion
{
    struct ResourceTypeEventLookup
    {
        usize pointer{};
        usize userData{};

        bool operator==(const ResourceTypeEventLookup& other) const
        {
            return this->pointer == other.pointer && this->userData == other.userData;
        }

        bool operator!=(const ResourceTypeEventLookup& other) const
        {
            return !((*this) == other);
        }
    };

    template<>
    struct Hash<ResourceTypeEventLookup>
    {
        constexpr static bool hasHash = true;

        constexpr static usize Value(const ResourceTypeEventLookup& value)
        {
            usize hash = 0;
            HashCombine(hash, value.pointer, value.userData);
            return hash;
        }
    };

    struct SubObjectSetData
    {
        HashSet<RID> subObjects{};
        HashSet<RID> prototypeRemoved{};
    };

    struct ResourceTypeEvent
    {
        VoidPtr userData{};
        ResourceEventType eventType{};
        FnResourceEvent event{};
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
        String simpleName;
        TypeID typeId;
        usize size;
        usize alignment;
        HashMap<String, SharedPtr<ResourceField>> fieldsByName;
        Array<ResourceField*> fieldsByIndex;
        HashMap<ResourceTypeEventLookup, ResourceTypeEvent> events;
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
        TypeHandler* typeHandler = nullptr;
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

        Logger& logger = Logger::GetLogger("Fyrion::Repository");
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
                std::unique_lock lock(pageMutex);
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
                std::unique_lock lock(byUUIDMutex);
                if (auto it = byUUID.Find(uuid))
                {
                    return it->second;
                }
            }
            RID rid = GetID();

            if (uuid)
            {
                std::unique_lock lock(byUUIDMutex);
                byUUID.Insert(uuid, rid);
            }
            return rid;
        }

        void UpdateVersion(ResourceStorage* resourceStorage)
        {
            ++resourceStorage->version;
            if (resourceStorage->parent)
            {
                UpdateVersion(resourceStorage->parent);
            }
        }

        void DestroyStorage(ResourceStorage* resourceStorage);

        void DestroyData(ResourceData* data, bool destroySubObjects)
        {
            if (data)
            {
                if (data->memory)
                {
                    for (int i = 0; i < data->fields.Size(); ++i)
                    {
                        if (data->fields[i] != nullptr)
                        {
                            if (data->storage->resourceType->fieldsByIndex[i]->fieldType == ResourceFieldType::SubObjectSet)
                            {
                                SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(data->fields[i]);
                                if (destroySubObjects)
                                {
                                    for (auto it: subObjectSetData.subObjects)
                                    {
                                        DestroyStorage(&pages[it.first.page]->elements[it.first.offset]);
                                    }
                                }
                                subObjectSetData.~SubObjectSetData();
                            }
                            else if (destroySubObjects && data->storage->resourceType->fieldsByIndex[i]->fieldType == ResourceFieldType::SubObject)
                            {
                                RID suboject = *static_cast<RID*>(data->fields[i]);
                                DestroyStorage(&pages[suboject.page]->elements[suboject.offset]);
                            }
                            else if (data->storage->resourceType->fieldsByIndex[i]->typeHandler)
                            {
                                data->storage->resourceType->fieldsByIndex[i]->typeHandler->Destructor(data->fields[i]);
                            }

                            data->fields[i] = nullptr;
                        }
                    }

                    if (data->storage->typeHandler)
                    {
                        data->storage->typeHandler->Destructor(data->memory);
                    }

                    allocator.MemFree(data->memory);
                    data->memory = nullptr;
                }
                allocator.DestroyAndFree(data);
            }
        }

        void DestroyStorage(ResourceStorage* resourceStorage)
        {
            resourceStorage->markedToDestroy = true;

            if (resourceStorage->resourceType)
            {
                for (auto itEvent: resourceStorage->resourceType->events)
                {
                    if ((itEvent.second.eventType && ResourceEventType::Destroy) != 0)
                    {
                        ResourceObject oldObject{resourceStorage->data, true};
                        ResourceObject newObject{nullptr, true};
                        itEvent.second.event(itEvent.second.userData, ResourceEventType::Destroy, oldObject, newObject);
                    }
                }
            }

            if (resourceStorage->data)
            {
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

        u64 GenerateBufferId()
        {
            return Random::Xorshift64star();
        }

        String GetBufferFile(StreamObject* streamObject)
        {

            if (!streamObject->MappedTo().Empty())
            {
                return streamObject->MappedTo();
            }
            else
            {
                char strBuffer[17]{};
                usize bufSize = U64ToHex(streamObject->GetBufferId(), strBuffer);
                return Path::Join(FileSystem::TempFolder(), StringView{strBuffer, bufSize});
            }
        }
    }


    void Repository::CreateResourceType(const ResourceTypeCreation& resourceTypeCreation)
    {
        SharedPtr<ResourceType> resourceType = MakeShared<ResourceType>();
        resourceType->name        = resourceTypeCreation.name;
        resourceType->simpleName  = resourceTypeCreation.simpleName;
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

        new(PlaceHolder(), resourceStorage) ResourceStorage{
            .rid = rid,
            .uuid = uuid,
            .data = {}
        };

        if (const auto it = resourceTypes.Find(typeId))
        {
            resourceStorage->resourceType = it->second.Get();
        }
        else if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            resourceStorage->typeHandler = typeHandler;
        }

        return rid;
    }

    ResourceObject Repository::Read(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return ResourceObject{storage->data, true};
    }

    ResourceObject Repository::ReadNoPrototypes(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return ResourceObject{storage->data, false};
    }

    ResourceObject Repository::Write(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        ResourceType* resourceType = storage->resourceType;

        FY_ASSERT(resourceType, "Resource type is null");

        ResourceData* data = allocator.Alloc<ResourceData>(storage);
        data->memory = allocator.MemAlloc(resourceType->size, 1);
        data->fields.Resize(resourceType->fieldsByIndex.Size());
        data->readOnly = false;

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
        return ResourceObject{data, true};
    }

    void Repository::DestroyResource(RID rid)
    {
        FY_ASSERT(rid, "resource cannot be null");
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
        ToDestroyResourceData data{};
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

    ResourceType* Repository::GetResourceTypeByName(const StringView& typeName)
    {
        if (auto it = resourceTypesByName.Find(typeName))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    TypeID Repository::GetResourceTypeId(ResourceType* resourceType)
    {
        return resourceType->typeId;
    }

    StringView Repository::GetResourceTypeName(ResourceType* resourceType)
    {
        return resourceType->name;
    }

    StringView Repository::GetResourceTypeSimpleName(ResourceType* resourceType)
    {
        if (!resourceType) return "";

        if (!resourceType->simpleName.Empty())
        {
            return resourceType->simpleName;
        }
        return resourceType->name;
    }

    void Repository::AddResourceTypeEvent(TypeID typeId, VoidPtr userData, ResourceEventType eventType, FnResourceEvent event)
    {
        if (auto it = resourceTypes.Find(typeId))
        {
            it->second->events.Emplace(
                ResourceTypeEventLookup{
                    .pointer = reinterpret_cast<usize>(event),
                    .userData = reinterpret_cast<usize>(userData)
                },
                ResourceTypeEvent{
                    .userData = userData,
                    .eventType = eventType,
                    .event = event
                });
        }
    }

    void Repository::RemoveResourceTypeEvent(TypeID typeId, VoidPtr userData, FnResourceEvent event)
    {
        if (auto it = resourceTypes.Find(typeId))
        {
            it->second->events.Erase(ResourceTypeEventLookup{
                .pointer = reinterpret_cast<usize>(event),
                .userData = reinterpret_cast<usize>(userData)
            });
        }
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
            std::unique_lock lock(byUUIDMutex);
            byUUID.Insert(uuid, rid);
        }
    }

    void Repository::SetPath(const RID& rid, const StringView& path)
    {
        std::unique_lock lockByPath(byPathMutex);
        byPath.Erase(path);
        byPath.Insert(path, rid);
    }

    void Repository::RemovePath(const StringView& path)
    {
        std::unique_lock lockByPath(byPathMutex);
        byPath.Erase(path);
    }

    UUID Repository::GetUUID(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->uuid;
    }

    RID Repository::GetPrototype(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        if (storage->prototype)
        {
            return storage->prototype->rid;
        }
        return {};
    }

    RID Repository::GetParent(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        if (storage->parent)
        {
            return storage->parent->rid;
        }
        return {};
    }

    RID Repository::GetByUUID(const UUID& uuid)
    {
        std::unique_lock lock(byUUIDMutex);

        if (auto it = byUUID.Find(uuid))
        {
            return it->second;
        }
        return {};
    }

    RID Repository::GetByPath(const StringView& path)
    {
        std::unique_lock lock(byPathMutex);

        if (auto it = byPath.Find(path))
        {
            return it->second;
        }
        return {};
    }

    TypeID Repository::GetResourceTypeID(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->resourceType ? storage->resourceType->typeId : 0;
    }

    ResourceType* Repository::GetResourceType(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->resourceType;
    }

    TypeHandler* Repository::GetResourceTypeHandler(const RID& rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->typeHandler;
    }

    RID Repository::GetOrCreateByUUID(const UUID& uuid)
    {
        return GetOrCreateByUUID(uuid, 0);
    }

    RID Repository::GetOrCreateByUUID(const UUID& uuid, TypeID typeId)
    {
        {
            std::unique_lock lock(byUUIDMutex);

            auto it = byUUID.Find(uuid);
            if (it != byUUID.end())
            {
                return it->second;
            }
        }

        RID rid = GetID();
        {
            std::unique_lock lock(byUUIDMutex);
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

    ConstPtr Repository::ReadData(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return storage->data.load()->memory;
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
        data->memory = storage->typeHandler->NewInstance(allocator);
        if (pointer)
        {
            storage->typeHandler->Copy(pointer, data->memory);
        }
        storage->data.store(data);
        UpdateVersion(storage);
    }

    ///*********************************************************************ResourceObject**************************************************************************************************************


    ResourceObject::ResourceObject(ResourceData* data, bool readPrototypes) : m_data(data), m_readPrototypes(readPrototypes)
    {
    }

    void ResourceObject::SetValue(u32 index, ConstPtr pointer)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        field->typeHandler->Copy(pointer, WriteValue(index));
    }

    VoidPtr ResourceObject::WriteValue(u32 index)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::Value, "Field is not ResourceFieldType::Value");
        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
            field->typeHandler->Construct(m_data->fields[index]);
        }
        return m_data->fields[index];
    }

    ConstPtr ResourceObject::GetValue(u32 index) const
    {
        ConstPtr ptr = m_data->fields[index];
        if (m_readPrototypes && !ptr && m_data->storage->prototype)
        {
            ResourceObject prototype{m_data->storage->prototype->data, m_readPrototypes};
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

    void ResourceObject::AddToSubObjectSet(u32 index, const Span<RID>& subObjects)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");

        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
            new(PlaceHolder(), m_data->fields[index]) SubObjectSetData();
        }

        SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);

        for (const RID& rid: subObjects)
        {
            subObjectSetData.subObjects.Insert(rid);
            ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
            storage->parent      = m_data->storage;
            storage->parentIndex = index;
        }
    }


    void ResourceObject::RemoveFromSubObjectSet(u32 index, RID subObject)
    {
        RemoveFromSubObjectSet(index, {&subObject, 1});
    }

    void ResourceObject::RemoveFromSubObjectSet(u32 index, const Span<RID>& subObjects)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");
        if (m_data->fields[index] != nullptr)
        {
            SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);

            for (const RID& rid: subObjects)
            {
                ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
                storage->parent = nullptr;
                storage->parentIndex = U32_MAX;
                subObjectSetData.subObjects.Erase(rid);
            }
        }
    }

    void ResourceObject::ClearSubObjectSet(u32 index)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");
        if (m_data->fields[index] != nullptr)
        {
            SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);
            for (auto& it: subObjectSetData.subObjects)
            {
                ResourceStorage* storage = &pages[it.first.page]->elements[it.first.offset];
                storage->parent = nullptr;
                storage->parentIndex = U32_MAX;
            }
            subObjectSetData.subObjects.Clear();
            if (subObjectSetData.prototypeRemoved.Empty())
            {
                subObjectSetData.~SubObjectSetData();
                m_data->fields[index] = nullptr;
            }
        }
    }

    usize ResourceObject::GetSubObjectSetCount(u32 index)
    {
        usize count{};
        ResourceGetSubObjectSet(m_data, nullptr, index, count, nullptr, m_readPrototypes);
        return count;
    }

    void ResourceObject::GetSubObjectSet(u32 index, Span<RID> subObjects)
    {
        usize count{};
        ResourceGetSubObjectSet(m_data, nullptr, index, count, &subObjects, m_readPrototypes);
    }

    usize ResourceObject::GetRemoveFromPrototypeSubObjectSetCount(u32 index) const
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");
        if (m_data->fields[index] == nullptr)
        {
            return 0;
        }
        SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);
        return subObjectSetData.prototypeRemoved.Size();
    }

    void ResourceObject::GetRemoveFromPrototypeSubObjectSet(u32 index, Span<RID> remove) const
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");
        if (m_data->fields[index] == nullptr)
        {
            return;
        }
        SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);
        u32 i = 0;

        for (const auto rid: subObjectSetData.prototypeRemoved)
        {
            remove[i++] = rid.first;
        }
    }

    void ResourceObject::RemoveFromPrototypeSubObjectSet(u32 index, RID remove)
    {
        RemoveFromPrototypeSubObjectSet(index, {&remove, 1});
    }

    void ResourceObject::RemoveFromPrototypeSubObjectSet(u32 index, const Span<RID>& remove)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");
        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
            new(PlaceHolder(), m_data->fields[index]) SubObjectSetData();
        }

        SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);
        for (const RID& rid: remove)
        {
            subObjectSetData.prototypeRemoved.Insert(rid);
        }
    }

    void ResourceObject::CancelRemoveFromPrototypeSubObjectSet(u32 index, RID remove)
    {
        CancelRemoveFromPrototypeSubObjectSet(index, {&remove, 1});
    }

    void ResourceObject::CancelRemoveFromPrototypeSubObjectSet(u32 index, const Span<RID>& remove)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::SubObjectSet, "Field is not ResourceFieldType::SubObjectSet");
        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
            new(PlaceHolder(), m_data->fields[index]) SubObjectSetData();
        }

        SubObjectSetData& subObjectSetData = *static_cast<SubObjectSetData*>(m_data->fields[index]);
        for (const RID& rid: remove)
        {
            subObjectSetData.prototypeRemoved.Erase(rid);
        }
    }

    bool ResourceObject::Has(u32 index) const
    {
        return GetValue(index) != nullptr;
    }

    Array<RID> ResourceObject::GetSubObjectSetAsArray(u32 index)
    {
        u32 count = GetSubObjectSetCount(index);
        Array<RID> rids{count};
        GetSubObjectSet(index, rids);
        return rids;
    }

    u32 ResourceObject::GetValueCount() const
    {
        return m_data->storage->resourceType->fieldsByIndex.Size();
    }

    u32 ResourceObject::GetIndex(const StringView& name) const
    {
        if (auto it = m_data->storage->resourceType->fieldsByName.Find(name))
        {
            return it->second->index;
        }
        return U32_MAX;
    }

    StringView ResourceObject::GetName(u32 index) const
    {
        return m_data->storage->resourceType->fieldsByIndex[index]->name;
    }

    TypeHandler* ResourceObject::GetFieldType(u32 index) const
    {
        return m_data->storage->resourceType->fieldsByIndex[index]->typeHandler;
    }

    ResourceFieldType ResourceObject::GetResourceType(u32 index) const
    {
        return m_data->storage->resourceType->fieldsByIndex[index]->fieldType;
    }

    ResourceObject::operator bool() const
    {
        return m_data != nullptr;
    }

    RID ResourceObject::GetRID() const
    {
        return m_data->storage->rid;
    }

    void ResourceObject::Commit()
    {
        m_data->readOnly = true;
        if (m_data->dataOnWrite)
        {
            if (m_data->storage->data.compare_exchange_strong(m_data->dataOnWrite, m_data))
            {
                for (auto itEvent: m_data->storage->resourceType->events)
                {
                    if ((itEvent.second.eventType && ResourceEventType::Update) != 0)
                    {
                        ResourceObject oldObject{m_data->dataOnWrite, true};
                        ResourceObject newObject{m_data, true};
                        itEvent.second.event(itEvent.second.userData, ResourceEventType::Update, oldObject, newObject);
                    }
                }

                UpdateVersion(m_data->storage);

                toCollectItems.enqueue(ToDestroyResourceData{
                    .data = m_data->dataOnWrite,
                    .destroySubObjects = false,
                    .destroyResource = false
                });
                m_data = nullptr;
            }
        }
        else
        {
            m_data->storage->data = m_data;

            for (auto itEvent: m_data->storage->resourceType->events)
            {
                if ((itEvent.second.eventType && ResourceEventType::Insert) != 0)
                {
                    ResourceObject oldObject{nullptr, true};
                    ResourceObject newObject{m_data, true};
                    itEvent.second.event(itEvent.second.userData, ResourceEventType::Insert, oldObject, newObject);
                }
            }
            UpdateVersion(m_data->storage);
        }
    }

    ResourceObject::~ResourceObject()
    {
        if (m_data && !m_data->readOnly)
        {
            DestroyData(m_data, true);
        }
    }

    bool ResourceObject::ResourceSubObjectAllowed(u32 index, ResourceData* data, ResourceData* ownerData, const RID& rid, bool checkPrototypes)
    {
        if (ownerData)
        {
            SubObjectSetData* subObjectSetData = static_cast<SubObjectSetData*>(ownerData->fields[index]);
            if (subObjectSetData && subObjectSetData->prototypeRemoved.Has(rid))
            {
                return false;
            }
        }

        if (data->storage->prototype && checkPrototypes)
        {
            return ResourceSubObjectAllowed(index, data->storage->prototype->data, data, rid, checkPrototypes);
        }

        return true;
    }

    void ResourceObject::ResourceGetSubObjectSet(ResourceData* data, ResourceData* ownerData, u32 index, usize& count, Span<RID>* subObjects, bool checkPrototypes)
    {
        SubObjectSetData* subObjectSetData = static_cast<SubObjectSetData*>(data->fields[index]);

        if (data->storage->prototype && checkPrototypes)
        {
            ResourceGetSubObjectSet(data->storage->prototype->data, data, index, count, subObjects, checkPrototypes);
        }

        if (subObjectSetData)
        {
            for (auto it: subObjectSetData->subObjects)
            {
                if (ResourceSubObjectAllowed(index, data, ownerData, it.first, checkPrototypes))
                {
                    if (subObjects)
                    {
                        (*subObjects)[count] = it.first;
                    }
                    count++;
                }
            }
        }
    }

    StreamObject* ResourceObject::GetStream(u32 index)
    {
        return (StreamObject*)GetValue(index);
    }

    StreamObject* ResourceObject::WriteStream(u32 index)
    {
        ResourceField* field = m_data->storage->resourceType->fieldsByIndex[index];
        FY_ASSERT(field->fieldType == ResourceFieldType::Stream, "Field is not ResourceFieldType::Stream");

        if (m_data->fields[index] == nullptr)
        {
            m_data->fields[index] = static_cast<char*>(m_data->memory) + field->offset;
            StreamObject* streamObject = new(PlaceHolder(), m_data->fields[index]) StreamObject{};
            streamObject->SetBufferId(GenerateBufferId());
        }

        return static_cast<StreamObject*>(m_data->fields[index]);
    }

    void StreamObject::MapTo(const StringView& file, usize offset)
    {
        m_mapFile = file;
    }

    StringView StreamObject::MappedTo()
    {
        return m_mapFile;
    }

    void StreamObject::Set(VoidPtr data, usize size)
    {
        m_mapFile.Clear();
        FileHandler fileHandler = FileSystem::OpenFile(GetBufferFile(this), AccessMode::WriteOnly);
        FileSystem::WriteFile(fileHandler, data, size);
        FileSystem::CloseFile(fileHandler);
    }

    usize StreamObject::Size()
    {
        return FileSystem::GetFileStatus(GetBufferFile(this)).fileSize;
    }

    void StreamObject::Get(VoidPtr data, usize size, usize offset)
    {
        FileHandler fileHandler = FileSystem::OpenFile(GetBufferFile(this), AccessMode::ReadOnly);
        FileSystem::ReadFile(fileHandler, data, size);
        FileSystem::CloseFile(fileHandler);
    }

    u64 StreamObject::GetBufferId() const
    {
        return m_id;
    }

    void StreamObject::SetBufferId(u64 bufferId)
    {
        m_id = bufferId;
    }

    void RepositoryInit()
    {
        Repository::CreateResource({});
    }

    void RepositoryShutdown()
    {
        Repository::GarbageCollect();

        for (u64 i = 0; i < counter; ++i)
        {
            ResourceStorage* storage = &pages[PAGE(i)]->elements[OFFSET(i)];
            DestroyData(storage->data, false);
            storage->~ResourceStorage();
        }

        for (u64 i = 0; i < pageCount; ++i)
        {
            if (!pages[i])
            {
                break;
            }
            allocator.MemFree(pages[i]);
            pages[i] = nullptr;
        }

        counter = 0;
        pageCount = 0;
        resourceTypes.Clear();
        resourceTypesByName.Clear();
        byUUID.Clear();
        byPath.Clear();
    }

    void RegisterResourceTypes()
    {
        Registry::Type<RID>();
        Registry::Type<Array<RID>>("Fyrion::RIDArray");
        Registry::Type<SubObjectSetData>();
        Registry::Type<StreamObject>();
    }
}
