#include "Serialization.hpp"
#include "Registry.hpp"
#include "yyjson.h"

namespace Fyrion
{
    ArchiveObject Serialization::Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, VoidPtr instance)
    {
        if (typeHandler == nullptr || instance == nullptr) return {};

        const ArchiveObject object = writer.CreateObject();
        for (FieldHandler* field : typeHandler->GetFields())
        {
            const TypeInfo& typeInfo = field->GetFieldInfo().typeInfo;

            if (FnArchiveWrite archiveWrite = typeInfo.archive.ArchiveWrite)
            {
                archiveWrite(writer, object, field->GetName(), field->GetFieldPointer(instance));
            }
            else if (typeInfo.apiId == GetTypeID<ArrayApi>())
            {
                ArchiveObject array = writer.CreateArray();

                VoidPtr arrayPtr = field->GetFieldPointer(instance);
                ArrayApi arrayApi{};
                typeInfo.extractApi(&arrayApi);
                usize    size = arrayApi.size(arrayPtr);
                TypeInfo itemInfo = arrayApi.getTypeInfo();

                bool empty = true;

                if (FnArchiveAdd archiveAdd = itemInfo.archive.ArchiveAdd)
                {
                    for (usize i = 0; i < size; ++i)
                    {
                        if (ConstPtr value = arrayApi.getConst(arrayPtr, i))
                        {
                            archiveAdd(writer, array, value);
                            empty = false;
                        }
                    }
                }
                else if (TypeHandler* itemHandler = Registry::FindTypeById(itemInfo.typeId))
                {
                    for (usize i = 0; i < size; ++i)
                    {
                        if (ArchiveObject value = Serialize(itemHandler, writer, arrayApi.get(arrayPtr, i)))
                        {
                            writer.AddValue(array, value);
                            empty = false;
                        }
                    }
                }

                if (!empty)
                {
                    writer.WriteValue(object, field->GetName(), array);
                }
            }
            else if (const TypeHandler* fieldType = Registry::FindTypeById(typeInfo.typeId))
            {
                writer.WriteValue(object, field->GetName(), Serialize(fieldType, writer, field->GetFieldPointer(instance)));
            }
        }
        return object;
    }

    void Serialization::Deserialize(const TypeHandler* typeHandler, ArchiveReader& reader, ArchiveObject object, VoidPtr instance)
    {
        if (typeHandler == nullptr || instance == nullptr) return;

        for (FieldHandler* field : typeHandler->GetFields())
        {
            const TypeInfo& typeInfo = field->GetFieldInfo().typeInfo;

            if (FnArchiveRead archiveRead = typeInfo.archive.ArchiveRead)
            {
                archiveRead(reader, object, field->GetName(), field->GetFieldPointer(instance));
            }
            else if (typeInfo.apiId == GetTypeID<ArrayApi>())
            {
                VoidPtr arrPtr = field->GetFieldPointer(instance);

                ArrayApi arrayApi{};
                typeInfo.extractApi(&arrayApi);

                arrayApi.clear(arrPtr);

                ArchiveObject arr = reader.ReadObject(object, field->GetName());
                usize         size = reader.ArrSize(arr);
                TypeInfo      itemInfo = arrayApi.getTypeInfo();
                ArchiveObject item{};

                if (FnArchiveGet archiveGet = itemInfo.archive.ArchiveGet)
                {
                    for (usize i = 0; i < size; ++i)
                    {
                        item = reader.Next(arr, item);
                        archiveGet(reader, item, arrayApi.pushNew(arrPtr));
                    }
                }
                else if (TypeHandler* itemHandler = Registry::FindTypeById(itemInfo.typeId))
                {
                    for (usize i = 0; i < size; ++i)
                    {
                        item = reader.Next(arr, item);
                        Deserialize(itemHandler, reader, item, arrayApi.pushNew(arrPtr));
                    }
                }
            }
            else if (const TypeHandler* fieldType = Registry::FindTypeById(typeInfo.typeId))
            {
                if (!field->GetFieldInfo().isPointer)
                {
                    Deserialize(fieldType, reader, reader.ReadObject(object, field->GetName()), field->GetFieldPointer(instance));
                }
            }
        }
    }

    void Serialization::WriteEnum(TypeID typeId, ArchiveWriter& writer, ArchiveObject object, StringView name, i64 value)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            if (ValueHandler* valueHandler = typeHandler->FindValueByCode(value))
            {
                writer.WriteString(object, name, valueHandler->GetDesc());
            }
        }
    }

    bool Serialization::ReadEnum(TypeID typeId, ArchiveReader& reader, ArchiveObject object, StringView name, i64& value)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            if (StringView desc = reader.ReadString(object, name); !desc.Empty())
            {
                if (ValueHandler* valueHandler = typeHandler->FindValueByName(desc))
                {
                    value = valueHandler->GetCode();
                    return true;
                }
            }
        }
        return false;
    }

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

    JsonAssetWriter::JsonAssetWriter(SerializationOptions serializationOptions) : serializationOptions(serializationOptions)
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
        // char buffer[20];
        // usize size = StringConverter<u64>::ToString(buffer, 0, value);
        // yyjson_mut_obj_add_strncpy(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), buffer, size);
        yyjson_mut_obj_add_uint(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value);
    }

    void JsonAssetWriter::WriteFloat(ArchiveObject object, const StringView& name, f64 value)
    {
        yyjson_mut_obj_add_real(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value);
    }

    void JsonAssetWriter::WriteString(ArchiveObject object, const StringView& name, const StringView& value)
    {
        yyjson_mut_obj_add_strncpy(doc, static_cast<yyjson_mut_val*>(object.handler), name.CStr(), value.CStr(), value.Size());
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

    void JsonAssetWriter::AddString(ArchiveObject array, const StringView& value)
    {
        yyjson_mut_arr_add_strncpy(doc, static_cast<yyjson_mut_val*>(array.handler), value.CStr(), value.Size());
    }

    void JsonAssetWriter::AddValue(ArchiveObject array, ArchiveObject value)
    {
        yyjson_mut_arr_add_val(static_cast<yyjson_mut_val*>(array.handler), static_cast<yyjson_mut_val*>(value.handler));
    }

    bool JsonAssetWriter::HasOpt(SerializationOptions option)
    {
        return serializationOptions && option;
    }

    String JsonAssetWriter::Stringify(ArchiveObject object)
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

    JsonAssetWriter::~JsonAssetWriter()
    {
        yyjson_mut_doc_free(doc);
    }

    JsonAssetReader::JsonAssetReader(StringView data)
    {
        FY_ASSERT(!data.Empty(), "data cannot be empty");
        const yyjson_read_flag flg = {};
        yyjson_read_err        err;
        doc = yyjson_read_opts(const_cast<char*>(data.begin()), data.Size(), flg, &alloc, &err);
        if (err.code != 0)
        {
            FY_ASSERT(false, "error");
        }
    }

    ArchiveObject JsonAssetReader::ReadObject()
    {
        return {yyjson_doc_get_root(doc)};
    }

    bool JsonAssetReader::ReadBool(ArchiveObject object, const StringView& name)
    {
        if (yyjson_val* val = yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size()))
        {
            return yyjson_get_bool(val);
        }
        return false;
    }

    i64 JsonAssetReader::ReadInt(ArchiveObject object, const StringView& name)
    {
        if (yyjson_val* val = yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size()))
        {
            return yyjson_get_int(val);
        }
        return 0;
    }

    u64 JsonAssetReader::ReadUInt(ArchiveObject object, const StringView& name)
    {
        if (yyjson_val* val = yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size()))
        {
            return yyjson_get_uint(val);
        }
        return 0;
    }

    StringView JsonAssetReader::ReadString(ArchiveObject object, const StringView& name)
    {
        if (yyjson_val* val = yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size()))
        {
            return yyjson_get_str(val);
        }
        return {};
    }

    f64 JsonAssetReader::ReadFloat(ArchiveObject object, const StringView& name)
    {
        if (yyjson_val* val = yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size()))
        {
            return yyjson_get_real(val);
        }
        return 0.0f;
    }

    ArchiveObject JsonAssetReader::ReadObject(ArchiveObject object, const StringView& name)
    {
        yyjson_val* val = yyjson_obj_getn(static_cast<yyjson_val*>(object.handler), name.CStr(), name.Size());
        return {val};
    }

    usize JsonAssetReader::ArrSize(ArchiveObject object)
    {
        return yyjson_arr_size(static_cast<yyjson_val*>(object.handler));
    }

    ArchiveObject JsonAssetReader::Next(ArchiveObject object, ArchiveObject item)
    {
        yyjson_val* arr = static_cast<yyjson_val*>(object.handler);
        if (item.handler == nullptr)
        {
            return {yyjson_arr_get_first(arr)};
        }

        return {unsafe_yyjson_get_next(static_cast<yyjson_val*>(item.handler))};
    }

    i64 JsonAssetReader::GetInt(ArchiveObject object)
    {
        return yyjson_get_int(static_cast<yyjson_val*>(object.handler));
    }

    u64 JsonAssetReader::GetUInt(ArchiveObject object)
    {
        return yyjson_get_uint(static_cast<yyjson_val*>(object.handler));
    }

    StringView JsonAssetReader::GetString(ArchiveObject object)
    {
        return yyjson_get_str(static_cast<yyjson_val*>(object.handler));
    }

    f64 JsonAssetReader::GetFloat(ArchiveObject object)
    {
        return yyjson_get_real(static_cast<yyjson_val*>(object.handler));
    }

    bool JsonAssetReader::GetBool(ArchiveObject object)
    {
        return yyjson_get_bool(static_cast<yyjson_val*>(object.handler));
    }

    JsonAssetReader::~JsonAssetReader()
    {
        yyjson_doc_free(doc);
    }
}
