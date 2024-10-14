#pragma once

#include "UUID.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion
{
    typedef void (*FnResourceLoader)(VoidPtr loaderData, VoidPtr instance, TypeHandler* typeHandler);
}

namespace Fyrion::Repository
{
    FY_API void         Register(UUID uuid, TypeID typeId, bool instantiate, VoidPtr loaderData, FnResourceLoader loader);
    FY_API VoidPtr      Load(UUID uuid);
    FY_API TypeHandler* GetTypeHandler(UUID uuid);

    template<typename T>
    T* Load(UUID uuid)
    {
        return static_cast<T*>(Load(uuid));
    }
}
