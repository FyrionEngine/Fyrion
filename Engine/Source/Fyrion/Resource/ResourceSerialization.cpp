#include "ResourceSerialization.hpp"

//TODO revisit this in the future

namespace Fyrion::ResourceSerialization
{
    void ParseObject(const StringView& buffer, VoidPtr instance, TypeHandler* typeHandler)
    {

    }

    void ParseResource(const StringView& buffer, RID rid)
    {

    }

    RID ParseResourceInfo(const StringView& buffer)
    {
        return RID();
    }

    ///********************************************************************************************************************************************
    ///*******************************************************************WRITER*******************************************************************
    ///********************************************************************************************************************************************

    struct WriterContext
    {
        String buffer{};
        String indentChar{" "};
        u32 indentNumber = 0;

        void AddIndentation()
        {
            indentNumber++;
        }

        void RemoveIndentation()
        {
            indentNumber--;
        }

        void Indent()
        {
            for (int i = 0; i < indentNumber; ++i)
            {
                buffer.Append(indentChar);
            }
        }
    };

    void WriteObject(WriterContext& context, VoidPtr instance, TypeHandler* handler);
    void WriteResource(WriterContext& context, RID rid);
    void WriteResourceInfo(WriterContext& context, RID rid);


    void WriteField(WriterContext& context, CPtr pointer, const TypeInfo& typeInfo, FieldHandler* fieldHandler)
    {
        if (typeInfo.TypeId == GetTypeID<RID>())
        {
            bool subobjectField = false;
            RID rid = *static_cast<const RID*>(pointer);
            context.buffer.Append("\"");
            char buffer[StringConverter<UUID>::BufferCount + 1] = {};
            StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(rid));
            context.buffer.Append(buffer);
            context.buffer.Append("\"");
        }
        else if (typeInfo.ToString && typeInfo.StringSize)
        {
            context.buffer.Append("\"");
            u32 size = typeInfo.StringSize(pointer);
            String str(size);
            typeInfo.ToString(pointer, str.begin());
            context.buffer.Reserve(context.buffer.Size() + size);
            for (const auto c: str)
            {
                if (c == '\"' || c == '\\')
                {
                    context.buffer.Append("\\");
                }
                context.buffer.Append(c);
            }
            context.buffer.Append("\"");
        }
        else if (typeInfo.ApiId == GetTypeID<ArrayApi>())
        {

            if (context.buffer[context.buffer.Size()-1] == '[')
            {
                context.buffer.Append("\n");
                context.Indent();
            }
            context.buffer.Append("[");
            ArrayApi arrayApi{};
            typeInfo.ExtractApi(&arrayApi);
            TypeInfo elementInfo = arrayApi.GetTypeInfo();
            bool added = false;
            for (int i = 0; i < arrayApi.Size(pointer); ++i)
            {
                WriteField(context, arrayApi.Get(pointer, i), elementInfo, nullptr);
                context.buffer.Append(",");
                added = true;
            }
            if (added)
            {
                context.buffer.Erase(context.buffer.end() - 1, context.buffer.end());
            }
            context.buffer.Append("]");
        }
        else
        {
            TypeHandler* typeHandler = Reflection::FindTypeById(typeInfo.TypeId);
            if (typeHandler)
            {
                context.AddIndentation();
                context.buffer.Append("{\n");
                WriteObject(context, pointer, typeHandler);
                context.RemoveIndentation();
                context.Indent();
                context.buffer.Append("}");
            }
        }
    }

    void WriteObject(WriterContext& context, CPtr instance, TypeHandler* handler)
    {
        Span<FieldHandler*> fields = handler->GetFields();
        for (FieldHandler* field: fields)
        {
            //if (field.HasAttribute<SerializationIgnore>()) continue;

            context.Indent();
            context.buffer.Append(field->GetName());
            context.buffer.Append(": ");
            TypeInfo typeInfo = field->GetFieldInfo().typeInfo;
            WriteField(context, field->GetFieldPointer(instance), typeInfo, field);
            context.buffer.Append("\n");
        }
    }    
    

    String WriteObject(VoidPtr instance, TypeHandler* handler)
    {
        FY_ASSERT(instance, "instance cannot be null");
        FY_ASSERT(handler, "handler cannot be null");
        WriterContext context{};
        WriteObject(context, instance, handler);
        return context.buffer;
    }

    void WriteResourceInfo(WriterContext& context, RID rid)
    {
        context.Indent();
        context.Buffer.Append("_uuid: ");
        char buffer[StringConverter<UUID>::BufferCount + 1] = {};
        StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(rid));
        context.Buffer.Append("\"");
        context.Buffer.Append(buffer);
        context.Buffer.Append("\"");
        context.Buffer.Append("\n");
        context.Indent();
        context.Buffer.Append("_type: ");
        context.Buffer.Append("\"");
        context.Buffer.Append(Repository::GetResourceTypeName(Repository::GetResourceType(rid)));
        context.Buffer.Append("\"");
        context.Buffer.Append("\n");

        RID prototype = Repository::GetPrototypeRID(rid);
        if (prototype)
        {
            context.Indent();
            context.Buffer.Append("_prototype: ");
            StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(prototype));
            context.Buffer.Append("\"");
            context.Buffer.Append(buffer);
            context.Buffer.Append("\"");
            context.Buffer.Append("\n");
        }

        if (!Repository::IsEmpty(rid))
        {
            context.Indent();
            context.Buffer.Append("_object: {\n");
            context.AddIndentation();
            context.AddIndentation();
            WriteResource(context, rid);
            context.RemoveIndentation();
            context.RemoveIndentation();
            context.Indent();
            context.Buffer.Append("}");
            context.Buffer.Append("\n");
        }
    }

    void WriteResource(WriterContext& context, RID rid)
    {
        ResourceObject object = Repository::ReadNoPrototypes(rid);
        u32 valueCount = object.GetValueCount();
        for (int i = 0; i < valueCount; ++i)
        {
            ResourceFieldType type = object.GetResourceType(i);
            StringView name = object.GetName(i);

            if (type == ResourceFieldType_Value)
            {
                ConstCPtr value = object.GetValue(i);
                if (!value) continue;

                context.Indent();
                context.Buffer.Append(name);
                context.Buffer.Append(": ");

                TypeHandlerPtr handler = object.GetFieldType(i);
                TypeInfo typeInfo = handler->GetTypeInfo();
                WriteField(context, (CPtr) value, typeInfo, nullptr);
                context.Buffer.Append("\n");
            }
            else if (type == ResourceFieldType_SubObject && object.Has(i))
            {
                RID subObject = object.GetSubObject(i);
                if (!subObject) continue;

                context.Indent();
                context.Buffer.Append(name);
                context.Buffer.Append(": ");
                context.Buffer.Append("{\n");
                context.AddIndentation();
                WriteResourceInfo(context, subObject);
                context.RemoveIndentation();
                context.Indent();
                context.Buffer.Append("}");
                context.Buffer.Append("\n");
            }
            else if (type == ResourceFieldType_Buffer && object.Has(i))
            {
                BufferObject* bufferObject = object.GetBuffer(i);
                char strBuffer[17]{};
                usize bufSize = U64ToHex(Buffer::GetBufferId(bufferObject), strBuffer);

                context.Indent();
                context.Buffer.Append(name);
                context.Buffer.Append(": ");
                context.Buffer.Append("\"");
                context.Buffer.Append(StringView{strBuffer, bufSize});
                context.Buffer.Append("\"");
                context.Buffer.Append("\n");
            }
            else if (type == ResourceFieldType_SubObjectSet)
            {
                u32 subObjectSetCount = object.GetSubObjectSetCount(i);
                u32 removedCount = object.GetRemoveFromPrototypeSubObjectSetCount(i);
                if (subObjectSetCount == 0 && removedCount == 0) continue;

                context.Indent();
                context.Buffer.Append(name);
                context.Buffer.Append(": ");
                context.Buffer.Append("{\n");
                context.AddIndentation();

                if (removedCount > 0)
                {
                    context.Indent();
                    context.Buffer.Append("_exclude");
                    context.Buffer.Append(": ");
                    context.Buffer.Append("[");
                    context.AddIndentation();

                    Array<RID> removedRids(removedCount);
                    object.GetRemoveFromPrototypeSubObjectSet(i, removedRids);

                    char buffer[StringConverter<UUID>::BufferCount + 1] = {};

                    for (RID removed: removedRids)
                    {
                        context.Buffer.Append("\"");
                        StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(removed));
                        context.Buffer.Append(buffer);
                        context.Buffer.Append("\",");
                    }
                    context.Buffer.Erase(context.Buffer.end() - 1, context.Buffer.end());
                    context.RemoveIndentation();
                    context.Buffer.Append("]\n");
                }

                if (subObjectSetCount > 0)
                {
                    Array<RID> subObjects(subObjectSetCount);
                    object.GetSubObjectSet(i, subObjects);

                    context.Indent();
                    context.Buffer.Append("_values");
                    context.Buffer.Append(": ");
                    context.Buffer.Append("[");
                    context.AddIndentation();

                    for(RID subobject: subObjects)
                    {
                        context.Buffer.Append("\n");
                        context.Indent();
                        context.Buffer.Append("{\n");
                        context.AddIndentation();
                        WriteResourceInfo(context, subobject);
                        context.RemoveIndentation();
                        context.Indent();
                        context.Buffer.Append("},");
                    }

                    context.Buffer.Erase(context.Buffer.end() - 1, context.Buffer.end());
                    context.Buffer.Append("\n");
                    context.RemoveIndentation();
                    context.Indent();
                    context.Buffer.Append("]\n");
                }

                context.RemoveIndentation();
                context.Indent();
                context.Buffer.Append("}");
                context.Buffer.Append("\n");
            }
        }
        if (valueCount == 0)
        {
            TypeID resourceType = Repository::GetResourceTypeID(rid);
            TypeHandler* typeHandler = Reflection::FindTypeById(resourceType);
            ConstCPtr obj = Repository::Read(rid, resourceType);
            if (typeHandler && obj)
            {
                WriteObject(context, (CPtr)obj, typeHandler);
            }
        }
    }

    String WriteResource(RID rid)
    {
        WriterContext context{};
        WriteResource(context, rid);
        return context.buffer;
    }

    String WriteResourceInfo(RID rid)
    {
        WriterContext context{};
        WriteResourceInfo(context, rid);
        return context.buffer;
    }

}