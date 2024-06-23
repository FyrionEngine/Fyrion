#pragma once

#include "Fyrion/Core/Serialization.hpp"
#include "Fyrion/Core/String.hpp"


typedef struct yyjson_mut_doc yyjson_mut_doc;
typedef struct yyjson_mut_val yyjson_mut_val;

namespace Fyrion
{
    class FY_API JsonAssetWriter : public ArchiveWriter
    {
    public:
        JsonAssetWriter();
        ~JsonAssetWriter() override;

        ArchiveObject CreateObject() override;
        ArchiveObject CreateArray() override;

        void WriteBool(ArchiveObject object, const StringView& name, bool value) override;
        void WriteInt(ArchiveObject object, const StringView& name, i64 value) override;
        void WriteUInt(ArchiveObject object, const StringView& name, u64 value) override;
        void WriteFloat(ArchiveObject object, const StringView& name, f64 value) override;
        void WriteUUID(ArchiveObject object, const StringView& name, const UUID& value) override;
        void WriteString(ArchiveObject object, const StringView& name, const StringView& value) override;
        void WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value) override;


        void AddBool(ArchiveObject array, bool value) override;
        void AddInt(ArchiveObject array, i64 value) override;
        void AddUInt(ArchiveObject array, u64 value) override;
        void AddFloat(ArchiveObject array, f64 value) override;
        void AddUUID(ArchiveObject array, const UUID& value) override;
        void AddString(ArchiveObject array, const StringView& value) override;
        void AddValue(ArchiveObject array, ArchiveObject value) override;


        static String Stringify(ArchiveObject object);

    private:
        yyjson_mut_doc* doc = nullptr;
    };
}
