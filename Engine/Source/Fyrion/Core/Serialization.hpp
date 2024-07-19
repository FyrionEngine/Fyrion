#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"


namespace Fyrion
{
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
        virtual bool          ReadBool(ArchiveObject object, const StringView& name) = 0;
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
        virtual bool          GetBool(ArchiveObject object) = 0;
    };

    typedef void (*FnArchiveWrite)(ArchiveWriter& writer, ArchiveObject object, StringView name, ConstPtr value);
    typedef void (*FnArchiveRead)(ArchiveReader& reader, ArchiveObject object, StringView name, VoidPtr value);
    typedef void (*FnArchiveAdd)(ArchiveWriter& writer, ArchiveObject array, ConstPtr value);
    typedef void (*FnArchiveGet)(ArchiveReader& reader, ArchiveObject item, VoidPtr value);


    template <typename T, typename Enable = void>
    struct ArchiveType
    {
        constexpr static bool hasArchiveImpl = false;
    };


#define FY_ARCHIVE_TYPE_IMPL(Name, T)      \
    template <>                     \
    struct ArchiveType<T>           \
    {                               \
        constexpr static bool hasArchiveImpl = true;    \
                                                        \
        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, const T* value) \
        { \
            if (*value) \
            { \
                writer.Write##Name(object, name, *value); \
            } \
        } \
        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, T* value) \
        { \
            *value = reader.Read##Name(object, name); \
        } \
            \
        static void Add(ArchiveWriter& writer, ArchiveObject array, const T* value) \
        { \
            writer.Add##Name(array, *value); \
        } \
 \
        static void Get(ArchiveReader& reader, ArchiveObject item, T* value) \
        { \
            *value =  reader.Get##Name(item); \
        } \
    };

    FY_ARCHIVE_TYPE_IMPL(Int, i8);
    FY_ARCHIVE_TYPE_IMPL(Int, i16);
    FY_ARCHIVE_TYPE_IMPL(Int, i32);
    FY_ARCHIVE_TYPE_IMPL(Int, i64);
    FY_ARCHIVE_TYPE_IMPL(UInt, u8);
    FY_ARCHIVE_TYPE_IMPL(UInt, u16);
    FY_ARCHIVE_TYPE_IMPL(UInt, u32);
    FY_ARCHIVE_TYPE_IMPL(UInt, u64);
    FY_ARCHIVE_TYPE_IMPL(Float, f32);
    FY_ARCHIVE_TYPE_IMPL(Float, f64);
    FY_ARCHIVE_TYPE_IMPL(Bool, bool);

    namespace Serialization
    {
        FY_API ArchiveObject Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, ConstPtr instance);
        FY_API void          Deserialize(const TypeHandler* typeHandler, ArchiveReader& reader, ArchiveObject object, VoidPtr instance);
    }
}
