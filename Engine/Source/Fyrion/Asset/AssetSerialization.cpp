#include "AssetSerialization.hpp"
#include "yyjson.h"

namespace Fyrion
{
    namespace
    {

        VoidPtr Malloc(VoidPtr ctx, usize size)
        {
            return MemoryGlobals::GetDefaultAllocator().MemAlloc(size, 1);
        }

        VoidPtr Realloc(VoidPtr ctx, VoidPtr ptr, usize oldSize, usize size)
        {
            return MemoryGlobals::GetDefaultAllocator().MemRealloc(ptr, size);
        }

        void Free(VoidPtr ctx, VoidPtr ptr)
        {
            MemoryGlobals::GetDefaultAllocator().MemFree(ptr);
        }

        yyjson_alc alloc{
            .malloc = Malloc,
            .realloc = Realloc,
            .free = Free,
            .ctx = nullptr
        };
    }

    JsonAssetWriter::JsonAssetWriter()
    {
        doc = yyjson_mut_doc_new(&alloc);
        root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
    }

    void JsonAssetWriter::WriteBool(const StringView& name, bool value)
    {
        yyjson_mut_obj_add_bool(doc, root, name.CStr(), value);
    }
    void JsonAssetWriter::WriteInt(const StringView& name, i64 value)
    {
        yyjson_mut_obj_add_int(doc, root, name.CStr(), value);
    }
    void JsonAssetWriter::WriteUInt(const StringView& name, u64 value)
    {
        yyjson_mut_obj_add_uint(doc, root, name.CStr(), value);
    }
    void JsonAssetWriter::WriteFloat(const StringView& name, f64 value)
    {
        yyjson_mut_obj_add_real(doc, root, name.CStr(), value);
    }
    void JsonAssetWriter::WriteUUID(const StringView& name, const UUID& value)
    {

    }
    void JsonAssetWriter::WriteString(const StringView& name, const StringView& value)
    {
        yyjson_mut_obj_add_strn(doc, root, name.CStr(), value.CStr(), value.Size());
    }

    String JsonAssetWriter::GetString() const
    {
        const yyjson_write_flag flg = YYJSON_WRITE_PRETTY | YYJSON_WRITE_ESCAPE_UNICODE;
        usize len = 0;
        yyjson_write_err err;

        if (char* json = yyjson_mut_write_opts(doc, flg, &alloc, &len, &err))
        {
            String ret = {json, len};
            Free(nullptr, json);
            return ret;
        }
        return{};
    }

    JsonAssetWriter::~JsonAssetWriter()
    {
        yyjson_mut_doc_free(doc);
    }
}
