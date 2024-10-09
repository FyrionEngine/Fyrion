#pragma once

#include "ResourceTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/TypeInfo.hpp"

namespace Fyrion::Repository
{
    FY_API RID      Create(TypeID typeId);
    FY_API ConstPtr Read(RID rid);
    FY_API void     Commit(RID rid, ConstPtr value);

    template<typename  T>
    const T& Read(RID rid)
    {
        return *static_cast<const T*>(Read(rid));
    }

    template <typename T>
    void Commit(RID rid, const T& value)
    {
        Commit(rid, &value);
    }

    template <typename T>
    RID Create()
    {
        return Create(GetTypeID<T>());
    }
}
