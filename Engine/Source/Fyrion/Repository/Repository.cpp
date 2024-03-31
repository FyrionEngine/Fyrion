#include "Repository.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/Core/HashSet.hpp"

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

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Repository", LogLevel::Debug);
        HashMap<TypeID, SharedPtr<ResourceType>> resourceTypes{};
        HashMap<String, SharedPtr<ResourceType>> resourceTypesByName{};
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
                resourceType->size += sizeof(RID);
            }
            else if (resourceFieldCreation.type == ResourceFieldType::SubObjectSet)
            {
                it->second->offset = resourceType->size;
                resourceType->size += sizeof(SubObjectSetData);
            }
            else if (resourceFieldCreation.type == ResourceFieldType::Stream)
            {
                it->second->offset = resourceType->size;
                resourceType->size += sizeof(StreamObject);
            }
        }

        resourceTypesByName.Insert(resourceTypeCreation.name, resourceType);
        resourceTypes.Emplace(resourceTypeCreation.typeId, Traits::Move(resourceType));

        logger.Debug("Resource Type {} Created", resourceTypeCreation.name);
    }

    RID Repository::CreateResource(TypeID typeId)
    {
        return RID();
    }

    RID Repository::CreateResource(TypeID typeId, const UUID& uuid)
    {
        return RID();
    }

    void RepositoryInit()
    {

    }

    void RepositoryShutdown()
    {
        resourceTypes.Clear();
    }
}