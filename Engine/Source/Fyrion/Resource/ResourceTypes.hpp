#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Hash.hpp"

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

        explicit operator bool() const noexcept
        {
            return this->offset > 0 || this->page > 0;
        }

        bool operator==(const RID& rid) const
        {
            return this->id == rid.id;
        }

        bool operator!=(const RID& rid) const
        {
            return !(*this == rid);
        }
    };

    template <>
    struct Hash<RID>
    {
        constexpr static bool hasHash = true;

        constexpr static usize Value(const RID& rid)
        {
            return Hash<u64>::Value(rid.id);
        }
    };


}
