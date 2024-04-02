#pragma once

#include <Fyrion/Common.hpp>
#include "Fyrion/Core/String.hpp"

namespace Fyrion
{

    class FY_API StreamObject
    {
    public:
        void        MapTo(const StringView& file, usize offset);
        StringView  MappedTo();
        void        Set(VoidPtr data, usize size);
        usize       Size();
        void        Get(VoidPtr data, usize size, usize offset);
        u64         GetBufferId();
        void        SetBufferId(u64 bufferId);
    private:
        u64         m_id{};
        String      m_mapFile{};
    };

}
