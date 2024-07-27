#include "Serialization.hpp"

#include "Registry.hpp"

namespace Fyrion
{
    ArchiveObject Serialization::Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, ConstPtr instance)
    {
        if (typeHandler == nullptr || instance == nullptr) return {};

        const ArchiveObject object = writer.CreateObject();
        for (FieldHandler* field : typeHandler->GetFields())
        {
            const TypeInfo& typeInfo = field->GetFieldInfo().typeInfo;

            if (FnArchiveWrite archiveWrite = typeInfo.archiveWrite)
            {
                archiveWrite(writer, object, field->GetName(), field->GetFieldPointer(instance));
            }
            else if (typeInfo.apiId == GetTypeID<ArrayApi>())
            {
                ArchiveObject array = writer.CreateArray();

                ConstPtr arrayPtr = field->GetFieldPointer(instance);
                ArrayApi arrayApi{};
                typeInfo.extractApi(&arrayApi);
                usize    size = arrayApi.size(arrayPtr);
                TypeInfo itemInfo = arrayApi.getTypeInfo();

                bool empty = true;

                if (FnArchiveAdd archiveAdd = itemInfo.archiveAdd)
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
                        if (ArchiveObject value = Serialize(itemHandler, writer, arrayApi.getConst(arrayPtr, i)))
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

            if (FnArchiveRead archiveRead = typeInfo.archiveRead)
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

                if (FnArchiveGet archiveGet = itemInfo.archiveGet)
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
}
