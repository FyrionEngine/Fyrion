#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    struct UUID;
    FY_HANDLER(ArchiveObject);

    struct ArchiveWriter
    {
        virtual ~ArchiveWriter() = default;

        virtual ArchiveObject CreateObject() = 0;
        virtual ArchiveObject CreateArray() = 0;

        virtual void WriteBool(ArchiveObject object, const StringView& name, bool value) = 0;
        virtual void WriteInt(ArchiveObject object, const StringView& name, i64 value) = 0;
        virtual void WriteUInt(ArchiveObject object, const StringView& name, u64 value) = 0;
        virtual void WriteFloat(ArchiveObject object, const StringView& name, f64 value) = 0;
        virtual void WriteUUID(ArchiveObject object, const StringView& name, const UUID& value) = 0;
        virtual void WriteString(ArchiveObject object, const StringView& name, const StringView& value) = 0;
        virtual void WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value) = 0;

        virtual void AddBool(ArchiveObject array, bool value) = 0;
        virtual void AddInt(ArchiveObject array, i64 value) = 0;
        virtual void AddUInt(ArchiveObject array, u64 value) = 0;
        virtual void AddFloat(ArchiveObject array, f64 value) = 0;
        virtual void AddUUID(ArchiveObject array, const UUID& value) = 0;
        virtual void AddString(ArchiveObject array, const StringView& value) = 0;
        virtual void AddValue(ArchiveObject array, ArchiveObject value) = 0;
    };


    template <typename T>
    struct ArchiveFieldHandler
    {
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)
        {
            FY_ASSERT(false, "ArchiveFieldHandler not implemented for this type");
        }
    };

    template <>
    struct ArchiveFieldHandler<u32>
    {
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const u32& value)
        {
            writer.WriteInt(object, name, value);
        }
    };
}
