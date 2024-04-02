#pragma once

#include "Fyrion/Core/StringView.hpp"
#include "ResourceTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion::ResourceSerialization
{
    FY_API void ParseObject(const StringView& buffer, VoidPtr instance, TypeHandler* typeHandler);
    FY_API void ParseResource(const StringView& buffer, RID rid);
    FY_API RID ParseResourceInfo(const StringView& buffer);

    FY_API String WriteObject(VoidPtr instance, TypeHandler* handler);
    FY_API String WriteResource(RID rid);
    FY_API String WriteResourceInfo(RID rid);
}