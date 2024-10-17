#pragma once

#include "String.hpp"
#include "TypeID.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

typedef struct yyjson_mut_doc yyjson_mut_doc;
typedef struct yyjson_mut_val yyjson_mut_val;
typedef struct yyjson_doc     yyjson_doc;

namespace Fyrion
{
    enum class SerializationOptions : u32
    {
        None                     = 0 << 1,
        IncludeNullOrEmptyValues = 1 << 1
    };

    ENUM_FLAGS(SerializationOptions, u32)

    struct FY_API ArchiveWriter
    {
        virtual ~ArchiveWriter() = default;

        virtual ArchiveValue CreateObject() = 0;
        virtual ArchiveValue CreateArray() = 0;

        virtual ArchiveValue BoolValue(bool value) = 0;
        virtual ArchiveValue IntValue(i64 value) = 0;
        virtual ArchiveValue UIntValue(u64 value) = 0;
        virtual ArchiveValue FloatValue(f64 value) = 0;
        virtual ArchiveValue StringValue(StringView value) = 0;

        virtual void AddToObject(ArchiveValue object, StringView name, ArchiveValue value) = 0;
        virtual void AddToArray(ArchiveValue array, ArchiveValue value) = 0;
    };

    struct ArchiveReader
    {
        virtual ~ArchiveReader() = default;

        virtual bool       BoolValue(ArchiveValue value) = 0;
        virtual i64        IntValue(ArchiveValue value) = 0;
        virtual u64        UIntValue(ArchiveValue value) = 0;
        virtual f64        FloatValue(ArchiveValue value) = 0;
        virtual StringView StringValue(ArchiveValue value) = 0;

        virtual ArchiveValue GetRootObject() = 0;
        virtual ArchiveValue GetObjectValue(ArchiveValue object, StringView name) = 0;

        virtual usize        ArraySize(ArchiveValue array) = 0;
        virtual ArchiveValue ArrayNext(ArchiveValue array, ArchiveValue item) = 0;
    };

#define FY_ARCHIVE_TYPE_IMPL(Name, T)      \
    template <>                            \
    struct ArchiveType<T>                  \
    {                                                   \
        constexpr static bool hasArchiveImpl = true;    \
                                                        \
        static ArchiveValue ToValue(ArchiveWriter& writer, const T& value)  \
        {                                       \
            return writer.Name##Value(value);   \
        }                                       \
                                                                                                \
        static void FromValue(ArchiveReader& reader, ArchiveValue archiveValue, T& typeValue)   \
        {                                                                                       \
            typeValue = reader.Name##Value(archiveValue);                                       \
        }                                                                                       \
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


    class FY_API JsonArchiveWriter : public ArchiveWriter
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(JsonArchiveWriter);
        FY_BASE_TYPES(ArchiveWriter);

        JsonArchiveWriter(SerializationOptions serializationOptions = SerializationOptions::None);
        ~JsonArchiveWriter() override;

        ArchiveValue CreateObject() override;
        ArchiveValue CreateArray() override;
        
        ArchiveValue BoolValue(bool value) override;
        ArchiveValue IntValue(i64 value) override;
        ArchiveValue UIntValue(u64 value) override;
        ArchiveValue FloatValue(f64 value) override;
        ArchiveValue StringValue(StringView value) override;

        void AddToObject(ArchiveValue object, StringView name, ArchiveValue value) override;
        void AddToArray(ArchiveValue array, ArchiveValue value) override;

        static String Stringify(ArchiveValue object);

    private:
        SerializationOptions serializationOptions = SerializationOptions::None;
        yyjson_mut_doc*      doc = nullptr;
    };


    class FY_API JsonArchiveReader : public ArchiveReader
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(JsonArchiveReader);
        FY_BASE_TYPES(ArchiveReader);

        JsonArchiveReader(StringView string);
        ~JsonArchiveReader() override;

        bool       BoolValue(ArchiveValue value) override;
        i64        IntValue(ArchiveValue value) override;
        u64        UIntValue(ArchiveValue value) override;
        f64        FloatValue(ArchiveValue value) override;
        StringView StringValue(ArchiveValue value) override;

        ArchiveValue GetRootObject() override;
        ArchiveValue GetObjectValue(ArchiveValue object, StringView name) override;

        usize        ArraySize(ArchiveValue array) override;
        ArchiveValue ArrayNext(ArchiveValue array, ArchiveValue item) override;

    private:
        yyjson_doc* doc = nullptr;
    };


    namespace Serialization
    {
        FY_API ArchiveValue Serialize(TypeID typeId, ArchiveWriter& writer, ConstPtr instance);
        FY_API ArchiveValue Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, ConstPtr instance);

        FY_API void Deserialize(const TypeHandler* typeHandler, ArchiveReader& reader, ArchiveValue object, VoidPtr instance);
        FY_API void Deserialize(TypeID typeId, ArchiveReader& reader, ArchiveValue object, VoidPtr instance);

        FY_API ArchiveValue EnumToValue(TypeID typeId, ArchiveWriter& writer, i64 value);
        FY_API void ValueToEnum(TypeID typeId, ArchiveReader& reader, ArchiveValue archiveValue, i64& value);
    }


    template <typename T>
    struct ArchiveType<T, Traits::EnableIf<Traits::IsEnum<T>>>
    {
        constexpr static bool hasArchiveImpl = true;

        static ArchiveValue ToValue(ArchiveWriter& writer, const T& value)
        {
            return Serialization::EnumToValue(GetTypeID<T>(), writer, static_cast<i64>(value));
        }

        static void FromValue(ArchiveReader& reader, ArchiveValue archiveValue, T& typeValue)
        {
            Serialization::ValueToEnum(GetTypeID<T>(), reader, archiveValue, *reinterpret_cast<i64*>(&typeValue));
        }
    };

}
