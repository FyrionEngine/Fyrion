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
        virtual void WriteString(ArchiveObject object, const StringView& name, const StringView& value) = 0;
        virtual void WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value) = 0;

        virtual void AddBool(ArchiveObject array, bool value) = 0;
        virtual void AddInt(ArchiveObject array, i64 value) = 0;
        virtual void AddUInt(ArchiveObject array, u64 value) = 0;
        virtual void AddFloat(ArchiveObject array, f64 value) = 0;
        virtual void AddString(ArchiveObject array, const StringView& value) = 0;
        virtual void AddValue(ArchiveObject array, ArchiveObject value) = 0;
    };

    struct ArchiveReader
    {
        virtual ~ArchiveReader() = default;

        virtual ArchiveObject ReadObject() = 0;
        virtual i64           ReadInt(ArchiveObject object, const StringView& name) = 0;
        virtual u64           ReadUInt(ArchiveObject object, const StringView& name) = 0;
        virtual StringView    ReadString(ArchiveObject object, const StringView& name) = 0;
        virtual f64           ReadFloat(ArchiveObject object, const StringView& name) = 0;
        virtual ArchiveObject ReadObject(ArchiveObject object, const StringView& name) = 0;

        virtual usize         ArrSize(ArchiveObject object) = 0;
        virtual ArchiveObject Next(ArchiveObject object, ArchiveObject item) = 0;
        virtual i64           GetInt(ArchiveObject object) = 0;
        virtual u64           GetUInt(ArchiveObject object) = 0;
        virtual StringView    GetString(ArchiveObject object) = 0;
        virtual f64           GetFloat(ArchiveObject object) = 0;
    };


    template <typename T, typename Enable = void>
    struct ArchiveType {};

#define FY_ARCHIVE_TYPE_INT(T)      \
    template<>                      \
    struct ArchiveType<T>           \
    {                               \
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)   \
        {                           \
            writer.WriteInt(object, name, value);   \
        }                                           \
                                                    \
        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, T& value)   \
        {                                                   \
            value = reader.ReadInt(object, name);            \
        }                                                   \
    }

#define FY_ARCHIVE_TYPE_UINT(T)      \
    template<>                      \
    struct ArchiveType<T>           \
    {                               \
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)   \
        {                           \
            writer.WriteUInt(object, name, value);      \
        }                                               \
                                                        \
        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, T& value)   \
        {                                                       \
            value = reader.ReadUInt(object, name);               \
        }       \
    }

#define FY_ARCHIVE_TYPE_FLOAT(T)      \
    template<>                      \
    struct ArchiveType<T>           \
    {                               \
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T& value)   \
        {                           \
            writer.WriteFloat(object, name, value);      \
        }                                               \
        \
        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, T& value)   \
        {                                                           \
            value = reader.ReadFloat(object, name);                  \
        }   \
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

    template <typename, typename = void>
    struct HasReadFieldImpl : Traits::FalseType {};

    template <typename T>
    struct HasReadFieldImpl<T, Traits::VoidType<decltype(static_cast<void(*)(ArchiveReader&, ArchiveObject, const StringView& name, T&)>(&ArchiveType<T>::ReadField))>> : Traits::TrueType {};

    template <typename T>
    constexpr bool HasReadField = HasReadFieldImpl<T>::value;
}
