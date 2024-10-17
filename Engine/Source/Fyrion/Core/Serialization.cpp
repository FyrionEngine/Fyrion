#include "Serialization.hpp"
#include "Registry.hpp"
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

    JsonArchiveWriter::JsonArchiveWriter(SerializationOptions serializationOptions) : serializationOptions(serializationOptions)
    {
        doc = yyjson_mut_doc_new(&alloc);
    }

    JsonArchiveWriter::~JsonArchiveWriter()
    {
        yyjson_mut_doc_free(doc);
    }

    ArchiveValue JsonArchiveWriter::CreateObject()
    {
        return {yyjson_mut_obj(doc)};
    }

    ArchiveValue JsonArchiveWriter::CreateArray()
    {
        return {yyjson_mut_arr(doc)};
    }

    ArchiveValue JsonArchiveWriter::BoolValue(bool value)
    {
        return {yyjson_mut_bool(doc, value)};
    }

    ArchiveValue JsonArchiveWriter::IntValue(i64 value)
    {
        return {yyjson_mut_uint(doc, value)};
    }

    ArchiveValue JsonArchiveWriter::UIntValue(u64 value)
    {
        return {yyjson_mut_uint(doc, value)};
    }

    ArchiveValue JsonArchiveWriter::FloatValue(f64 value)
    {
        return {yyjson_mut_real(doc, value)};
    }

    ArchiveValue JsonArchiveWriter::StringValue(StringView value)
    {
        return {yyjson_mut_strcpy(doc, value.CStr())};
    }

    void JsonArchiveWriter::AddToObject(ArchiveValue object, StringView name, ArchiveValue value)
    {
        yyjson_mut_obj_add_val(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), static_cast<yyjson_mut_val*>(value.handler));
    }

    void JsonArchiveWriter::AddToArray(ArchiveValue array, ArchiveValue value)
    {
        yyjson_mut_arr_add_val(static_cast<yyjson_mut_val*>(array.handler), static_cast<yyjson_mut_val*>(value.handler));
    }

    String JsonArchiveWriter::Stringify(ArchiveValue object)
    {
        if (!object) return {};

        const yyjson_write_flag flg = YYJSON_WRITE_PRETTY | YYJSON_WRITE_ESCAPE_UNICODE;
        usize                   len = 0;
        yyjson_write_err        err;

        if (char* json = yyjson_mut_val_write_opts(static_cast<yyjson_mut_val*>(object.handler), flg, &alloc, &len, &err))
        {
            String ret = {json, len};
            Free(nullptr, json);
            return ret;
        }

        if (err.code != 0)
        {
            FY_ASSERT(false, "error");
        }
        return {};
    }


    JsonArchiveReader::JsonArchiveReader(StringView string)
    {
        FY_ASSERT(!string.Empty(), "data cannot be empty");
        const yyjson_read_flag flg = {};
        yyjson_read_err        err;
        doc = yyjson_read_opts(const_cast<char*>(string.begin()), string.Size(), flg, &alloc, &err);
        if (err.code != 0)
        {
            FY_ASSERT(false, "error");
        }
    }

    JsonArchiveReader::~JsonArchiveReader()
    {
        yyjson_doc_free(doc);
    }

    bool JsonArchiveReader::BoolValue(ArchiveValue value)
    {
        if(value)
        {
            return yyjson_get_bool(static_cast<yyjson_val*>(value.handler));
        }
        return false;
    }

    i64 JsonArchiveReader::IntValue(ArchiveValue value)
    {
        if (value)
        {
            return yyjson_get_int(static_cast<yyjson_val*>(value.handler));
        }
        return 0;
    }

    u64 JsonArchiveReader::UIntValue(ArchiveValue value)
    {
        if (value)
        {
            return yyjson_get_uint(static_cast<yyjson_val*>(value.handler));
        }
        return 0;
    }

    f64 JsonArchiveReader::FloatValue(ArchiveValue value)
    {
        if (value)
        {
            return yyjson_get_real(static_cast<yyjson_val*>(value.handler));
        }
        return 0.0;
    }

    StringView JsonArchiveReader::StringValue(ArchiveValue value)
    {
        if (value)
        {
            return StringView{yyjson_get_str(static_cast<yyjson_val*>(value.handler))};
        }
        return "";
    }

    ArchiveValue JsonArchiveReader::GetRootObject()
    {
        return {yyjson_doc_get_root(doc)};
    }

    ArchiveValue JsonArchiveReader::GetObjectValue(ArchiveValue object, StringView name)
    {
        return {yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size())};
    }

    usize JsonArchiveReader::ArraySize(ArchiveValue array)
    {
        return yyjson_arr_size(static_cast<yyjson_val*>(array.handler));
    }

    ArchiveValue JsonArchiveReader::ArrayNext(ArchiveValue array, ArchiveValue item)
    {
        yyjson_val* arr = static_cast<yyjson_val*>(array.handler);
        if (item.handler == nullptr)
        {
            return {yyjson_arr_get_first(arr)};
        }

        return {unsafe_yyjson_get_next(static_cast<yyjson_val*>(item.handler))};
    }

    ArchiveValue Serialization::Serialize(TypeID typeId, ArchiveWriter& writer, ConstPtr instance)
    {
        return Serialize(Registry::FindTypeById(typeId), writer, instance);
    }

    ArchiveValue Serialization::Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, ConstPtr instance)
    {
        if (typeHandler == nullptr || instance == nullptr) return {};

        if (typeHandler->GetTypeInfo().archiveToValue)
        {
            return typeHandler->GetTypeInfo().archiveToValue(writer, instance);
        }

        ArchiveValue object = writer.CreateObject();

        for (FieldHandler* field : typeHandler->GetFields())
        {
            if (field->GetFieldInfo().typeInfo.archiveToValue)
            {
                if (ArchiveValue value = field->GetFieldInfo().typeInfo.archiveToValue(writer, field->GetFieldPointer(instance)))
                {
                    writer.AddToObject(object, field->GetName(), value);
                }
            }
            else if (const TypeHandler* fieldTypeHandler = Registry::FindTypeById(field->GetFieldInfo().typeInfo.typeId))
            {
                if (ArchiveValue value = Serialize(fieldTypeHandler, writer, field->GetFieldPointer(instance)))
                {
                    writer.AddToObject(object, field->GetName(), value);
                }
            }
        }

        return object;
    }

    void Serialization::Deserialize(const TypeHandler* typeHandler, ArchiveReader& reader, ArchiveValue object, VoidPtr instance)
    {
        if (typeHandler == nullptr || instance == nullptr) return;

        if (typeHandler->GetTypeInfo().archiveFromValue)
        {
            typeHandler->GetTypeInfo().archiveFromValue(reader, object, instance);
            return;
        }

        for (FieldHandler* field : typeHandler->GetFields())
        {
            ArchiveValue value = reader.GetObjectValue(object, field->GetName());

            if (field->GetFieldInfo().typeInfo.archiveFromValue)
            {
                field->GetFieldInfo().typeInfo.archiveFromValue(reader, value, field->GetFieldPointer(instance));
            }
            else if (const TypeHandler* fieldTypeHandler = Registry::FindTypeById(field->GetFieldInfo().typeInfo.typeId))
            {
                Deserialize(fieldTypeHandler, reader, value, field->GetFieldPointer(instance));
            }
        }
    }

    void Serialization::Deserialize(TypeID typeId, ArchiveReader& reader, ArchiveValue object, VoidPtr instance)
    {
        Deserialize(Registry::FindTypeById(typeId), reader, object, instance);
    }

    ArchiveValue Serialization::EnumToValue(TypeID typeId, ArchiveWriter& writer, i64 value)
    {
        if (const TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            if (ValueHandler* valueHandler = typeHandler->FindValueByCode(value))
            {
                return writer.StringValue(valueHandler->GetDesc());
            }
        }
        return {};
    }

    void Serialization::ValueToEnum(TypeID typeId, ArchiveReader& reader, ArchiveValue archiveValue, i64& value)
    {
        if (const TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            if (ValueHandler* valueHandler = typeHandler->FindValueByName(reader.StringValue(archiveValue)))
            {
                value = valueHandler->GetCode();
            }
        }
    }
}
