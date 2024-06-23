#pragma once

#include "Fyrion/Core/Serialization.hpp"


typedef struct yyjson_mut_doc yyjson_mut_doc;
typedef struct yyjson_mut_val yyjson_mut_val;

namespace Fyrion
{
    class FY_API JsonAssetWriter : public ArchiveWriter
    {
    public:
        JsonAssetWriter();
        ~JsonAssetWriter() override;

        void WriteBool(const StringView& name, bool value) override;
        void WriteInt(const StringView& name, i64 value) override;
        void WriteUInt(const StringView& name, u64 value) override;
        void WriteFloat(const StringView& name, f64 value) override;
        void WriteUUID(const StringView& name, const UUID& value) override;
        void WriteString(const StringView& name, const StringView& value) override;


        String GetString() const;

    private:
        yyjson_mut_doc* doc = nullptr;
        yyjson_mut_val* root = nullptr;
    };
}
