#pragma once

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Serialization.hpp"
#include "Fyrion/Core/String.hpp"


typedef struct yyjson_mut_doc yyjson_mut_doc;
typedef struct yyjson_mut_val yyjson_mut_val;
typedef struct yyjson_doc     yyjson_doc;

namespace Fyrion
{
    class FY_API JsonAssetWriter : public ArchiveWriter
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(JsonAssetWriter);
        FY_BASE_TYPES(ArchiveWriter);

        JsonAssetWriter(SerializationOptions serializationOptions = SerializationOptions::None);
        ~JsonAssetWriter() override;

        ArchiveObject CreateObject() override;
        ArchiveObject CreateArray() override;

        void WriteBool(ArchiveObject object, const StringView& name, bool value) override;
        void WriteInt(ArchiveObject object, const StringView& name, i64 value) override;
        void WriteUInt(ArchiveObject object, const StringView& name, u64 value) override;
        void WriteFloat(ArchiveObject object, const StringView& name, f64 value) override;
        void WriteString(ArchiveObject object, const StringView& name, const StringView& value) override;
        void WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value) override;


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
