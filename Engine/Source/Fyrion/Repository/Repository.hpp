#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/TypeInfo.hpp"
#include "ResourceTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    namespace Repository
    {
        FY_API void             CreateResourceType(const ResourceTypeCreation& resourceTypeCreation);

        FY_API RID              CreateResource(TypeID typeId);
        FY_API RID              CreateResource(TypeID typeId, const UUID& uuid);


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

    }
}

#include "ResourceBuilder.hpp"