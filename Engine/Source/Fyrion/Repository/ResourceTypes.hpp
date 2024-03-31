#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{

    struct RID
    {
        union
        {
            struct
            {
                u32 offset;
                u32 page;
            };
            u64 id{};
        };

        inline explicit operator bool() const noexcept
        {
            return this->offset > 0 || this->page > 0;
        }

        inline bool operator==(const RID& rid) const
        {
            return this->id == rid.id;
        }

        inline bool operator!=(const RID& rid) const
        {
            return !((*this) == rid);
        }
    };

    template<>
    struct Hash<RID>
    {
        constexpr static bool hasHash = true;
        constexpr static usize Value(const RID& rid)
        {
            return Hash<u64>::Value(rid.id);
        }
    };

    enum class ResourceFieldType
    {
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