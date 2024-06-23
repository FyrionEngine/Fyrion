#pragma once

#include "UUID.hpp"
#include "Fyrion/Common.hpp"


namespace Fyrion
{
    struct ArchiveWriter
    {
        virtual ~ArchiveWriter() = default;

        virtual void WriteBool(const StringView& name, bool value) = 0;
        virtual void WriteInt(const StringView& name, i64 value) = 0;
        virtual void WriteUInt(const StringView& name, u64 value) = 0;
        virtual void WriteFloat(const StringView& name, f64 value) = 0;
        virtual void WriteUUID(const StringView& name, const UUID& value) = 0;
        virtual void WriteString(const StringView& name, const StringView& value) = 0;
    };
}
