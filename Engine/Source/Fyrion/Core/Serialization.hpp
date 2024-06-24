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
    struct ArchiveType {};

#define FY_ARCHIVE_TYPE_INT(T)      \
    template<>                      \
    struct ArchiveType<T>           \
    {                               \
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)   \
        {                           \
            writer.WriteInt(object, name, value);   \
        }                                           \
    }

#define FY_ARCHIVE_TYPE_UINT(T)      \
    template<>                      \
    struct ArchiveType<T>           \
    {                               \
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)   \
        {                           \
            writer.WriteUInt(object, name, value);      \
        }                                               \
    }

#define FY_ARCHIVE_TYPE_FLOAT(T)      \
    template<>                      \
    struct ArchiveType<T>           \
    {                               \
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)   \
        {                           \
            writer.WriteFloat(object, name, value);      \
        }                                               \
    }


    FY_ARCHIVE_TYPE_INT(i8);
    FY_ARCHIVE_TYPE_INT(i16);
    FY_ARCHIVE_TYPE_INT(i32);
    FY_ARCHIVE_TYPE_INT(i64);

    FY_ARCHIVE_TYPE_UINT(u8);
    FY_ARCHIVE_TYPE_UINT(u16);
    FY_ARCHIVE_TYPE_UINT(u32);
    FY_ARCHIVE_TYPE_UINT(u64);

    FY_ARCHIVE_TYPE_FLOAT(f32);
    FY_ARCHIVE_TYPE_FLOAT(f64);

    template <typename, typename = void>
    struct HasWriteTypeImpl : Traits::FalseType {};

    template <typename T>
    struct HasWriteTypeImpl<T, Traits::VoidType<decltype(static_cast<void(*)(ArchiveWriter&, ArchiveObject, const T&)>(&ArchiveType<T>::WriteType))>> : Traits::TrueType {};

    template <typename T>
    constexpr bool HasWriteType = HasWriteTypeImpl<T>::value;


    template <typename, typename = void>
    struct HasWriteFieldImpl : Traits::FalseType {};

    template <typename T>
    struct HasWriteFieldImpl<T, Traits::VoidType<decltype(static_cast<void(*)(ArchiveWriter&, ArchiveObject, const StringView& name, const T&)>(&ArchiveType<T>::WriteField))>> : Traits::TrueType {};

    template <typename T>
    constexpr bool HasWriteField = HasWriteFieldImpl<T>::value;
}
