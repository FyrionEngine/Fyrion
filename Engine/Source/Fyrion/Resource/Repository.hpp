#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/TypeInfo.hpp"
#include "ResourceTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "ResourceObject.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    namespace Repository
    {
        FY_API void             CreateResourceType(const ResourceTypeCreation& resourceTypeCreation);
        FY_API TypeID           GetResourceTypeID(const StringView& typeName);
        FY_API TypeHandler*     GetResourceTypeHandler(ResourceType* resourceType);
        FY_API StringView       GetResourceTypeName(ResourceType* resourceType);
        FY_API bool             IsInstanced(ResourceType* resourceType);

        FY_API RID              CreateResource(TypeID typeId);
        FY_API RID              CreateResource(TypeID typeId, const UUID& uuid);
        FY_API RID              CreateFromPrototype(RID prototype);
        FY_API RID              CreateFromPrototype(RID prototype, const UUID& uuid);
        FY_API void             SetUUID(const RID& rid, const UUID& uuid);
        FY_API void             SetPath(const RID& rid, const StringView& path);
        FY_API void             RemovePath(const StringView& path);
        FY_API UUID             GetUUID(const RID& rid);
        FY_API RID              GetPrototypeRID(const RID& rid);
        FY_API RID              GetByUUID(const UUID& uuid);
        FY_API RID              GetByPath(const StringView& path);
        FY_API TypeID           GetResourceTypeID(const RID& rid);
        FY_API ResourceType*    GetResourceType(const RID& rid);
        FY_API RID              GetOrCreateByUUID(const UUID& uuid);
        FY_API RID              GetOrCreateByUUID(const UUID& uuid, TypeID typeId);
        FY_API void             ClearValues(RID rid);
        FY_API void             DestroyResource(RID rid);
        FY_API RID              CloneResource(RID rid);
        FY_API ResourceObject   Read(RID rid);
        FY_API ConstPtr         Read(RID rid, TypeID typeId);
        FY_API ResourceObject   Write(RID rid);
        FY_API void             Commit(RID rid, ConstPtr pointer);
        FY_API void             InactiveResource(RID rid);
        FY_API bool             IsActive(RID rid);
        FY_API bool             IsAlive(RID rid);
        FY_API bool             IsEmpty(RID rid);
        FY_API u32              GetVersion(RID rid);

        FY_API void             GarbageCollect();


        template<typename T>
        RID CreateResource()
        {
            return CreateResource(GetTypeID<T>());
        }

        template<typename T>
        RID CreateResource(const UUID& uuid)
        {
            return CreateResource(GetTypeID<T>(), uuid);
        }

        template<typename T>
        const T& Read(RID rid)
        {
            return *static_cast<const T*>(Read(rid, GetTypeID<T>()));
        }

    }
}

#include "ResourceBuilder.hpp"