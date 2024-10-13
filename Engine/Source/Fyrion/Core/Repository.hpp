#pragma once

#include "UUID.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion::Repository
{
    typedef void (*FnResourceLoader)(VoidPtr loaderData, VoidPtr instance, TypeHandler* typeHandler);

    FY_API void         Register(UUID uuid, TypeID typeId, bool instantiate, VoidPtr loaderData, FnResourceLoader loader);
    FY_API VoidPtr      Load(UUID uuid);
    FY_API TypeHandler* GetTypeHandler(UUID uuid);
}
