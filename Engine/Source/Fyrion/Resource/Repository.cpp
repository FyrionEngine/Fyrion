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
        HashMap<String, SharedPtr<ResourceField>> fieldsByName;
        Array<ResourceField*> fieldsByIndex;
        TypeHandler* typeHandler;
    };

    struct ResourceStorage
    {
        RID rid{};
        UUID uuid{};
        ResourceType* resourceType{};
        std::atomic<ResourceObject*> object{};
        ResourceStorage* prototype{};
        ResourceStorage* parent{};
        usize parentIndex = U32_MAX;
        bool markedToDestroy{};
        bool active = true;
        std::atomic<u32> version = 1;
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

        void DestroyResourceStorage(ResourceStorage* resourceStorage)
        {
            if (resourceStorage->object)
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

                allocator.DestroyAndFree(resourceStorage->object.load());
            }
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
            .object = {}
        };

        return rid;
    }

    ResourceObject& Repository::Read(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return *storage->object;
    }

    ResourceObject& Repository::Write(RID rid)
    {
        ResourceStorage* storage = &pages[rid.page]->elements[rid.offset];
        return *allocator.Alloc<ResourceObject>(storage);
    }

    void Repository::DestroyResource(RID rid)
    {
        //TODO
    }

    void Repository::GargageCollect()
    {
        //TODO
    }

    ResourceObject::ResourceObject(ResourceStorage* storage) : m_storage(storage)
    {
        ResourceType* resourceType = m_storage->resourceType;

        m_data = allocator.MemAlloc(resourceType->size, 1);
        m_fields.Resize(resourceType->fieldsByIndex.Size());

        if (storage->object)
        {
            ResourceObject* object = storage->object.load();
            m_dataOnWrite = object;

            for (int i = 0; i < resourceType->fieldsByIndex.Size(); ++i)
            {
                if (object->m_fields[i] != nullptr)
                {
                    m_fields[i] = static_cast<char*>(m_data) + resourceType->fieldsByIndex[i]->offset;
                    if (resourceType->fieldsByIndex[i]->typeHandler)
                    {
                        resourceType->fieldsByIndex[i]->typeHandler->Copy(object->m_fields[i], m_fields[i]);
                    }
                }
            }
        }
    }

    void ResourceObject::Commit()
    {

    }

    void ResourceObject::Rollback()
    {
        allocator.DestroyAndFree(this);
    }

    ResourceObject::~ResourceObject()
    {
        for (int i = 0; i < m_fields.Size(); ++i)
        {
            if (m_fields[i] != nullptr)
            {
                m_storage->resourceType->fieldsByIndex[i]->typeHandler->Destructor(m_fields[i]);
                m_fields[i] = nullptr;
            }
        }
        allocator.MemFree(m_data);
    }

    void ResourceObject::SetValue(u32 index, ConstPtr pointer)
    {
        ResourceField* field = m_storage->resourceType->fieldsByIndex[index];

        if (m_fields[index] == nullptr)
        {
            m_fields[index] = static_cast<char*>(m_data) + field->offset;
        }
        field->typeHandler->Copy(pointer, m_fields[index]);
    }

    ConstPtr ResourceObject::GetValue(u32 index)
    {
        return m_fields[index];
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
