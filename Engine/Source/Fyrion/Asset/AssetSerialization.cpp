#include "AssetSerialization.hpp"
#include "yyjson.h"
#include "Fyrion/Core/UUID.hpp"

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
    }

    ArchiveObject JsonAssetWriter::CreateObject()
    {
        return {yyjson_mut_obj(doc)};
    }

    ArchiveObject JsonAssetWriter::CreateArray()
    {
        return {yyjson_mut_arr(doc)};
    }

    void JsonAssetWriter::WriteBool(ArchiveObject object, const StringView& name, bool value)
    {
        yyjson_mut_obj_add_bool(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value);
    }

    void JsonAssetWriter::WriteInt(ArchiveObject object, const StringView& name, i64 value)
    {
        yyjson_mut_obj_add_int(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value);
    }

    void JsonAssetWriter::WriteUInt(ArchiveObject object, const StringView& name, u64 value)
    {
        yyjson_mut_obj_add_uint(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value);
    }

    void JsonAssetWriter::WriteFloat(ArchiveObject object, const StringView& name, f64 value)
    {
        yyjson_mut_obj_add_real(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value);
    }

    void JsonAssetWriter::WriteUUID(ArchiveObject object, const StringView& name, const UUID& value)
    {
        char buffer[StringConverter<UUID>::bufferCount] = {};
        StringConverter<UUID>::ToString(buffer, 0, value);
        yyjson_mut_obj_add_strncpy(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), buffer, StringConverter<UUID>::bufferCount);
    }

    void JsonAssetWriter::WriteString(ArchiveObject object, const StringView& name, const StringView& value)
    {
        yyjson_mut_obj_add_strn(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value.CStr(), value.Size());
    }

    void JsonAssetWriter::WriteValue(ArchiveObject object, const StringView& name, ArchiveObject value)
    {
        yyjson_mut_obj_add_val(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), static_cast<yyjson_mut_val*>(value.handler));
    }

    void JsonAssetWriter::AddBool(ArchiveObject array, bool value)
    {
        yyjson_mut_arr_add_bool(doc, static_cast<yyjson_mut_val*>(array.handler), value);
    }

    void JsonAssetWriter::AddInt(ArchiveObject array, i64 value)
    {
        yyjson_mut_arr_add_int(doc, static_cast<yyjson_mut_val*>(array.handler), value);
    }

    void JsonAssetWriter::AddUInt(ArchiveObject array, u64 value)
    {
        yyjson_mut_arr_add_uint(doc, static_cast<yyjson_mut_val*>(array.handler), value);
    }

    void JsonAssetWriter::AddFloat(ArchiveObject array, f64 value)
    {
        yyjson_mut_arr_add_real(doc, static_cast<yyjson_mut_val*>(array.handler), value);
    }

    void JsonAssetWriter::AddUUID(ArchiveObject array, const UUID& value)
    {
        char buffer[StringConverter<UUID>::bufferCount];
        StringConverter<UUID>::ToString(buffer, 0, value);
        yyjson_mut_arr_add_strncpy(doc, static_cast<yyjson_mut_val*>(array.handler), buffer, StringConverter<UUID>::bufferCount);
    }

    void JsonAssetWriter::AddString(ArchiveObject array, const StringView& value)
    {
        yyjson_mut_arr_add_strn(doc, static_cast<yyjson_mut_val*>(array.handler), value.CStr(), value.Size());
    }

    void JsonAssetWriter::AddValue(ArchiveObject array, ArchiveObject value)
    {
        yyjson_mut_arr_add_val(static_cast<yyjson_mut_val*>(array.handler), static_cast<yyjson_mut_val*>(value.handler));
    }

    String JsonAssetWriter::Stringify(ArchiveObject object)
    {
        const yyjson_write_flag flg = YYJSON_WRITE_PRETTY | YYJSON_WRITE_ESCAPE_UNICODE;
        usize                   len = 0;
        yyjson_write_err        err;

        if (char* json = yyjson_mut_val_write_opts(static_cast<yyjson_mut_val*>(object.handler), flg, &alloc, &len, &err))
        {
            String ret = {json, len};
            Free(nullptr, json);
            return ret;
        }
        return {};
    }

    JsonAssetWriter::~JsonAssetWriter()
    {
        yyjson_mut_doc_free(doc);
    }
}
