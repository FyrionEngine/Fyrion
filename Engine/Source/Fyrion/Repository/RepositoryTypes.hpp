#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{

    enum class ResourceFieldType
    {
        Undefined = 0,
        Value = 1,
        SubObject = 2,
        SubObjectSet = 3,
        Stream = 4
    };

    struct ResourceFieldCreation
    {
        u32 index{U32_MAX};
        StringView name{};
        ResourceFieldType type{};
        TypeID valueId{};
    };

    struct ResourceTypeCreation
    {
        StringView name{};
        TypeID typeId{};
        Span<ResourceFieldCreation> fields{};
    };

}