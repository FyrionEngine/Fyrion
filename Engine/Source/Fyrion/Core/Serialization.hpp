#pragma once

#include "Span.hpp"
#include "String.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/IO/FileTypes.hpp"

typedef struct yyjson_mut_doc yyjson_mut_doc;
typedef struct yyjson_mut_val yyjson_mut_val;
typedef struct yyjson_doc     yyjson_doc;

namespace Fyrion
{
    FY_HANDLER(ArchiveObject);

    enum class SerializationOptions : u32
    {
        None                     = 0 << 1,
        IncludeNullOrEmptyValues = 1 << 1
    };

    ENUM_FLAGS(SerializationOptions, u32)

    struct ArchiveWriter
    {
        virtual ~ArchiveWriter() = default;

        virtual ArchiveObject CreateObject() = 0;
        virtual ArchiveObject CreateArray() = 0;

        virtual void  WriteBool(ArchiveObject object, const StringView& name, bool value) = 0;
        virtual void  WriteInt(ArchiveObject object, const StringView& name, i64 value) = 0;
        virtual void  WriteUInt(ArchiveObject object, const StringView& name, u64 value) = 0;
        virtual void  WriteFloat(ArchiveObject object, const StringView& name, f64 value) = 0;
        virtual void  WriteString(ArchiveObject object, const StringView& name, const StringView& value) = 0;
        virtual void  WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value) = 0;
        virtual usize WriteStream(ArchiveObject object, const StringView& name, Span<u8> data) = 0;

        virtual void AddBool(ArchiveObject array, bool value) = 0;
        virtual void AddInt(ArchiveObject array, i64 value) = 0;
        virtual void AddUInt(ArchiveObject array, u64 value) = 0;
        virtual void AddFloat(ArchiveObject array, f64 value) = 0;
        virtual void AddString(ArchiveObject array, const StringView& value) = 0;
        virtual void AddValue(ArchiveObject array, ArchiveObject value) = 0;

        virtual bool HasOpt(SerializationOptions serializationOptions) = 0;
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

    typedef void (*FnArchiveWrite)(ArchiveWriter& writer, ArchiveObject object, StringView name, VoidPtr value);
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
    }

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

    FY_ARCHIVE_TYPE_IMPL(String, String);


    namespace Serialization
    {
        FY_API ArchiveObject Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, VoidPtr instance);
        FY_API void          Deserialize(const TypeHandler* typeHandler, ArchiveReader& reader, ArchiveObject object, VoidPtr instance);
        FY_API void          WriteEnum(TypeID typeId, ArchiveWriter& writer, ArchiveObject object, StringView name, i64 value);
        FY_API bool          ReadEnum(TypeID typeId, ArchiveReader& reader, ArchiveObject object, StringView name, i64& value);
    }

    template <typename T>
    struct ArchiveType<T, Traits::EnableIf<Traits::IsEnum<T>>>
    {
        constexpr static bool hasArchiveImpl = true;

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, const T* value)
        {
            Serialization::WriteEnum(GetTypeID<T>(), writer, object, name, static_cast<i64>(*value));
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, T* value)
        {
            i64 intValue = 0;
            if (Serialization::ReadEnum(GetTypeID<T>(), reader, object, name, intValue))
            {
                *value = static_cast<T>(intValue);
            }
        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, const T* value)
        {
            FY_ASSERT(false, "not implemented yet");
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, T* value)
        {
            FY_ASSERT(false, "not implemented yet");
        }
    };


    class FY_API JsonAssetWriter : public ArchiveWriter
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(JsonAssetWriter);
        FY_BASE_TYPES(ArchiveWriter);

        JsonAssetWriter(SerializationOptions serializationOptions = SerializationOptions::None, OutputFileStream* fileStream = nullptr);
        ~JsonAssetWriter() override;

        ArchiveObject CreateObject() override;
        ArchiveObject CreateArray() override;

        void     WriteBool(ArchiveObject object, const StringView& name, bool value) override;
        void     WriteInt(ArchiveObject object, const StringView& name, i64 value) override;
        void     WriteUInt(ArchiveObject object, const StringView& name, u64 value) override;
        void     WriteFloat(ArchiveObject object, const StringView& name, f64 value) override;
        void     WriteString(ArchiveObject object, const StringView& name, const StringView& value) override;
        void     WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value) override;
        ConstPtr WriteStream(ArchiveObject object, const StringView& name, Span<u8> data) override;


        void AddBool(ArchiveObject array, bool value) override;
        void AddInt(ArchiveObject array, i64 value) override;
        void AddUInt(ArchiveObject array, u64 value) override;
        void AddFloat(ArchiveObject array, f64 value) override;
        void AddString(ArchiveObject array, const StringView& value) override;
        void AddValue(ArchiveObject array, ArchiveObject value) override;

        bool HasOpt(SerializationOptions option) override;

        static String Stringify(ArchiveObject object);

    private:
        SerializationOptions serializationOptions = SerializationOptions::None;
        yyjson_mut_doc*      doc = nullptr;
        OutputFileStream*    fileStream = nullptr;
    };


    class FY_API JsonAssetReader : public ArchiveReader
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(JsonAssetReader);
        FY_BASE_TYPES(ArchiveReader);

        explicit JsonAssetReader(StringView data);
        ~JsonAssetReader() override;

        ArchiveObject ReadObject() override;

        bool          ReadBool(ArchiveObject object, const StringView& name) override;
        i64           ReadInt(ArchiveObject object, const StringView& name) override;
        u64           ReadUInt(ArchiveObject object, const StringView& name) override;
        StringView    ReadString(ArchiveObject object, const StringView& name) override;
        f64           ReadFloat(ArchiveObject object, const StringView& name) override;
        ArchiveObject ReadObject(ArchiveObject object, const StringView& name) override;

        usize         ArrSize(ArchiveObject object) override;
        ArchiveObject Next(ArchiveObject object, ArchiveObject item) override;
        i64           GetInt(ArchiveObject object) override;
        u64           GetUInt(ArchiveObject object) override;
        StringView    GetString(ArchiveObject object) override;
        f64           GetFloat(ArchiveObject object) override;
        bool          GetBool(ArchiveObject object) override;

    private:
        yyjson_doc* doc = nullptr;
    };
}
