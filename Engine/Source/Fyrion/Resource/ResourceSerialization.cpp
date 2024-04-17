#include "ResourceSerialization.hpp"
#include "Repository.hpp"

//TODO revisit this in the future

namespace Fyrion::ResourceSerialization
{
    struct ParserContext
    {
        StringView buffer{};
        String     identifier{};
        String     value{};
        usize      Pos{};
    };

    void ParseObject(ParserContext& context, VoidPtr instance, TypeHandler* typeHandler);
    void ParseResource(ParserContext& context, RID rid);

    void RemoveSpaces(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        while (c == ' ' || c == '\n' || c == '\r' || c == '\t')
        {
            context.Pos++;
            if (context.Pos >= context.buffer.Size()) return;
            c = context.buffer[context.Pos];
        }
    }

    void EndObject(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        while (c != '}')
        {
            context.Pos++;
            if (context.Pos >= context.buffer.Size()) return;
            c = context.buffer[context.Pos];
        }
        context.Pos++;
    }

    bool CheckString(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        if (c != '\"')
        {
            return false;
        }
        context.Pos++;
        c = context.buffer[context.Pos];
        while (c != '\"')
        {
            context.value.Append(c);
            context.Pos++;
            if (context.Pos >= context.buffer.Size()) return true;
            c = context.buffer[context.Pos];

            //scape char
            if (c == '\\')
            {
                context.Pos++; //ignore scape
                if (context.Pos >= context.buffer.Size()) return true;

                c = context.buffer[context.Pos];
                context.value.Append(c);
                context.Pos++;
                if (context.Pos >= context.buffer.Size()) return true;
                c = context.buffer[context.Pos];
            }

        }
        context.Pos++;
        return true;
    }

    bool CheckIdentifier(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        while (c != ':')
        {
            if (c != ' ')
            {
                context.identifier.Append(c);
            }
            context.Pos++;
            if (context.Pos >= context.buffer.Size()) return !context.identifier.Empty();
            c = context.buffer[context.Pos];
        }
        context.Pos++;
        return !context.identifier.Empty();
    }

    bool CheckObject(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        if (c != '{')
        {
            return false;
        }
        context.Pos++;
        return true;
    }

    bool CheckArray(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        if (c != '[')
        {
            return false;
        }
        context.Pos++;
        return true;
    }

    bool CheckText(ParserContext& context)
    {
        auto c = context.buffer[context.Pos];
        auto cn = context.buffer[context.Pos + 1];
        if (c != '[' || cn != '[')
        {
            return false;
        }
        context.Pos += 2;
        return true;
    }

    char Current(ParserContext& context)
    {
        if (context.Pos >= context.buffer.Size()) return '\3';
        return context.buffer[context.Pos];
    }

    void ParseText(ParserContext& context)
    {
        if (context.Pos + 2 >= context.buffer.Size()) return;

        auto c = context.buffer[context.Pos];
        auto cn = context.buffer[context.Pos + 1];
        bool isEnd = c == ']' && cn == ']';

        while (!isEnd)
        {
            context.value.Append(c);
            context.Pos++;

            if (context.Pos + 2 >= context.buffer.Size()) return;

            c = context.buffer[context.Pos];
            cn = context.buffer[context.Pos + 1];
            isEnd = c == ']' && cn == ']';
        }
        context.Pos += 2;
    }

    void ParseArray(ParserContext& context, VoidPtr pointer, const TypeInfo& typeInfo)
    {
        ArrayApi arrayApi{};
        TypeInfo elementInfo{};
        FnFromString fromString{};

        if (typeInfo.apiId == GetTypeID<ArrayApi>())
        {
            typeInfo.extractApi(&arrayApi);
            elementInfo = arrayApi.getTypeInfo();
            fromString = elementInfo.fromString;
        }

        RemoveSpaces(context);

        auto c = context.buffer[context.Pos];
        while (c != ']')
        {
            context.value.Clear();
            if (CheckString(context))
            {
                if (elementInfo.typeId == GetTypeID<RID>())
                {
                    if (pointer)
                    {
                        VoidPtr newElement = arrayApi.pushNew(pointer);
                        new(PlaceHolder(), newElement) RID{Repository::GetOrCreateByUUID(UUID::FromString(context.value))};
                    }
                }
                else if (fromString)
                {
                    if (pointer)
                    {
                        fromString(arrayApi.pushNew(pointer), context.value);
                    }
                }
            }
            else if (CheckArray(context))
            {
                VoidPtr newPointer = nullptr;
                if (pointer)
                {
                    newPointer = arrayApi.pushNew(pointer);
                }
                ParseArray(context, newPointer, elementInfo);
            }
            else if (CheckObject(context))
            {
                TypeHandler* objectType = nullptr;
                VoidPtr objectInstance = nullptr;
                if (pointer)
                {
                    objectType = Registry::FindTypeById(elementInfo.typeId);
                    objectInstance = arrayApi.pushNew(pointer);
                }
                ParseObject(context, objectInstance, objectType);
            }
            else
            {
                context.Pos++;
            }

            c = context.buffer[context.Pos];
        }
        context.Pos++;
    }

    void ParseObject(ParserContext& context, VoidPtr instance, TypeHandler* typeHandler)
    {
        while (context.Pos < context.buffer.Size())
        {
            RemoveSpaces(context);
            auto c = context.buffer[context.Pos];
            if (c == '}')
            {
                context.Pos++;
                break;
            }
            context.identifier.Clear();
            if (CheckIdentifier(context))
            {
                FieldHandler* field = nullptr;

                if (typeHandler)
                {
                    field = typeHandler->FindField(context.identifier);
                }

                RemoveSpaces(context);

                context.value.Clear();

                if (CheckString(context))
                {
                    if (field && instance)
                    {
                        FieldInfo fieldInfo = field->GetFieldInfo();
                        if (fieldInfo.typeInfo.typeId == GetTypeID<RID>())
                        {
                            RID rid = Repository::GetOrCreateByUUID(UUID::FromString(context.value));
                            field->SetValue(instance, &rid);
                        }
                        else if (fieldInfo.typeInfo.fromString != nullptr)
                        {

                            fieldInfo.typeInfo.fromString(field->GetFieldPointer(instance), context.value);
                        }
                    }
                }
                else if (CheckText(context))
                {
                    ParseText(context);
                    if (field && instance)
                    {
                        FieldInfo fieldInfo = field->GetFieldInfo();
                        if (fieldInfo.typeInfo.fromString != nullptr)
                        {

                            fieldInfo.typeInfo.fromString(field->GetFieldPointer(instance), context.value);
                        }
                    }
                }
                else if (CheckArray(context))
                {
                    TypeInfo typeInfo{};
                    VoidPtr pointer = nullptr;
                    if (field && instance)
                    {
                        typeInfo = field->GetFieldInfo().typeInfo;
                        pointer = field->GetFieldPointer(instance);
                    }
                    ParseArray(context, pointer, typeInfo);
                }
                else if (CheckObject(context))
                {
                    TypeHandler* objectType = nullptr;
                    VoidPtr objectInstance = nullptr;
                    if (field && instance)
                    {
                        objectType = Registry::FindTypeById(field->GetFieldInfo().typeInfo.typeId);
                        objectInstance = field->GetFieldPointer(instance);
                    }
                    ParseObject(context, objectInstance, objectType);
                }
            }
        }
    }


    void ParseObject(const StringView& buffer, VoidPtr instance, TypeHandler* typeHandler)
    {
        ParserContext parseContext{.buffer = buffer};
        ParseObject(parseContext, instance, typeHandler);
    }

    RID ParseResourceInfo(ParserContext& context)
    {
        RemoveSpaces(context);
        auto c = context.buffer[context.Pos];
        if (c == '}')
        {
            return {};
        }

        UUID objectUUID = {};
        TypeID typeId = {};

        //uuid
        context.identifier.Clear();
        CheckIdentifier(context);
        FY_ASSERT(context.identifier == "_uuid", "for subobjects, first item should be _uuid");
        RemoveSpaces(context);
        context.value.Clear();
        CheckString(context);
        objectUUID = UUID::FromString(context.value);

        context.identifier.Clear();
        RemoveSpaces(context);
        CheckIdentifier(context);
        FY_ASSERT(context.identifier == "_type", "for subobjects, second item should be _type");
        RemoveSpaces(context);
        context.value.Clear();
        CheckString(context);
        typeId = Repository::GetResourceTypeID(context.value);

        context.identifier.Clear();
        RemoveSpaces(context);
        CheckIdentifier(context);
        RemoveSpaces(context);

        if (context.identifier == "_prototype")
        {
            context.value.Clear();
            CheckString(context);
            RemoveSpaces(context);
            RID prototype = Repository::GetOrCreateByUUID(UUID::FromString(context.value), typeId);
            RID rid = Repository::CreateFromPrototype(prototype, objectUUID);

            //no overrides
            if (context.buffer[context.Pos] == '}')
            {
                context.Pos++;
                return rid;
            }

            context.identifier.Clear();
            RemoveSpaces(context);
            CheckIdentifier(context);
            RemoveSpaces(context);

            if (CheckObject(context))
            {
                ParseResource(context, rid);
                RemoveSpaces(context);
                if (context.buffer[context.Pos] == '}')
                {
                    context.Pos++;
                }
            }

            return rid;
        }
        else if (CheckObject(context))
        {
            RID rid = Repository::CreateResource(typeId, objectUUID);
            ParseResource(context, rid);
            RemoveSpaces(context);
            if (context.buffer[context.Pos] == '}')
            {
                context.Pos++;
            }
            return rid;
        }
        return{};
    }

    void ParseSubobjectSet(ParserContext& context, u32 index, ResourceObject& parent)
    {
        RemoveSpaces(context);
        auto c = context.buffer[context.Pos];
        if (c == '}')
        {
            context.Pos++;
            return;
        }

        context.identifier.Clear();
        CheckIdentifier(context);
        RemoveSpaces(context);

        if (context.identifier == "_exclude")
        {
            if (CheckArray(context))
            {
                RemoveSpaces(context);
                while (c != ']')
                {
                    if (CheckString(context))
                    {
                        RID rid = Repository::GetOrCreateByUUID(UUID::FromString(context.value));
                        parent.RemoveFromPrototypeSubObjectSet(index,rid);
                    }
                    else
                    {
                        context.Pos++;
                    }
                    c = context.buffer[context.Pos];
                }
                context.Pos++;
            }

            context.identifier.Clear();
            CheckIdentifier(context);
            RemoveSpaces(context);
        }

        if (CheckArray(context))
        {
            RemoveSpaces(context);
            c = context.buffer[context.Pos];
            while (c != ']')
            {
                if (CheckObject(context))
                {
                    RID suboject = ParseResourceInfo(context);
                    parent.AddToSubObjectSet(index, suboject);
                }
                else
                {
                    context.Pos++;
                }
                c = context.buffer[context.Pos];
            }
            context.Pos++;
        }
        RemoveSpaces(context);
        if (context.buffer[context.Pos] == '}')
        {
            context.Pos++;
        }

    }

    void ParseResource(ParserContext& context, RID rid)
    {
        ResourceType* resourceType = Repository::GetResourceType(rid);
        if (resourceType == nullptr) return;

        if (!Repository::IsResourceTypeData(resourceType))
        {
            ResourceObject object = Repository::Write(rid);
            while (context.Pos < context.buffer.Size())
            {
                RemoveSpaces(context);
                auto c = context.buffer[context.Pos];
                if (c == '}')
                {
                    context.Pos++;
                    break;
                }
                context.identifier.Clear();
                if (CheckIdentifier(context))
                {
                    RemoveSpaces(context);
                    context.value.Clear();


                    u32 index = object.GetIndex(context.identifier);
                    ResourceFieldType type = ResourceFieldType::Undefined;

                    TypeInfo typeInfo = {};
                    if (index != U32_MAX)
                    {
                        type = object.GetResourceType(index);
                        TypeHandler* typeHandler = object.GetFieldType(index);
                        if (typeHandler != nullptr)
                        {
                            typeInfo = typeHandler->GetTypeInfo();
                        }
                    }

                    if (CheckString(context))
                    {
                        if (type == ResourceFieldType::Stream)
                        {
                            StreamObject* stream = object.WriteStream(index);
                            stream->SetBufferId(HexTo64(StringView{context.value}));
                        }
                        else if (typeInfo.typeId == GetTypeID<RID>())
                        {
                            object.SetValue(index, Repository::GetOrCreateByUUID(UUID::FromString(context.value)));
                        }
                        else if (typeInfo.fromString)
                        {
                            VoidPtr value = object.WriteValue(index);
                            typeInfo.fromString(value, context.value);
                        }
                    }
                    else if (CheckText(context))
                    {
                        ParseText(context);
                        if (typeInfo.fromString)
                        {
                            VoidPtr value = object.WriteValue(index);
                            typeInfo.fromString(value, context.value);
                        }
                    }
                    else if (CheckArray(context))
                    {
                        if (type == ResourceFieldType::Value || type == ResourceFieldType::Undefined)
                        {
                            if (typeInfo.typeId == GetTypeID<Array<RID>>())
                            {
                                VoidPtr value = object.WriteValue(index);
                                ParseArray(context, value, typeInfo);
                            }
                        }
                    }
                    else if (CheckObject(context))
                    {
                        if (type == ResourceFieldType::Value || type == ResourceFieldType::Undefined)
                        {
                            TypeHandler* objectType = Registry::FindTypeById(typeInfo.typeId);
                            VoidPtr objectInstance = index != U32_MAX ? object.WriteValue(index) : nullptr;
                            ParseObject(context, objectInstance, objectType);
                        }
                        else if (type == ResourceFieldType::SubObject)
                        {
                            object.SetSubObject(index, ParseResourceInfo(context));
                        }
                        else if (type == ResourceFieldType::SubObjectSet)
                        {
                            ParseSubobjectSet(context, index, object);
                        }
                    }
                }
            }
            object.Commit();
        }
        else
        {
            TypeHandler* typeHandler = Repository::GetResourceTypeHandler(resourceType);
            if (typeHandler)
            {
                VoidPtr instance = typeHandler->NewInstance();
                ParseObject(context, instance, typeHandler);
                Repository::Commit(rid, instance);
                typeHandler->Destroy(instance);
            }
        }

    }


    void ParseResource(const StringView& buffer, RID rid)
    {
        ParserContext parseContext{.buffer = buffer};
        ParseResource(parseContext, rid);
    }

    RID ParseResourceInfo(const StringView& buffer)
    {
        ParserContext parseContext{.buffer = buffer};
        return ParseResourceInfo(parseContext);
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


    void WriteField(WriterContext& context, VoidPtr pointer, const TypeInfo& typeInfo, FieldHandler* fieldHandler)
    {
        if (typeInfo.typeId == GetTypeID<RID>())
        {
            bool subobjectField = false;
            RID rid = *static_cast<const RID*>(pointer);
            context.buffer.Append("\"");
            char buffer[StringConverter<UUID>::bufferCount + 1] = {};
            StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(rid));
            context.buffer.Append(buffer);
            context.buffer.Append("\"");
        }
        else if (typeInfo.toString && typeInfo.stringSize)
        {
            context.buffer.Append("\"");
            u32 size = typeInfo.stringSize(pointer);
            String str(size);
            typeInfo.toString(pointer, str.begin());
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
        else if (typeInfo.apiId == GetTypeID<ArrayApi>())
        {

            if (context.buffer[context.buffer.Size()-1] == '[')
            {
                context.buffer.Append("\n");
                context.Indent();
            }
            context.buffer.Append("[");
            ArrayApi arrayApi{};
            typeInfo.extractApi(&arrayApi);
            TypeInfo elementInfo = arrayApi.getTypeInfo();
            bool added = false;
            for (int i = 0; i < arrayApi.size(pointer); ++i)
            {
                WriteField(context, arrayApi.get(pointer, i), elementInfo, nullptr);
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
            TypeHandler* typeHandler = Registry::FindTypeById(typeInfo.typeId);
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

    void WriteObject(WriterContext& context, VoidPtr instance, TypeHandler* handler)
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
        context.buffer.Append("_uuid: ");
        char buffer[StringConverter<UUID>::bufferCount + 1] = {};
        StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(rid));
        context.buffer.Append("\"");
        context.buffer.Append(buffer);
        context.buffer.Append("\"");
        context.buffer.Append("\n");
        context.Indent();
        context.buffer.Append("_type: ");
        context.buffer.Append("\"");
        context.buffer.Append(Repository::GetResourceTypeName(Repository::GetResourceType(rid)));
        context.buffer.Append("\"");
        context.buffer.Append("\n");

        RID prototype = Repository::GetPrototypeRID(rid);
        if (prototype)
        {
            context.Indent();
            context.buffer.Append("_prototype: ");
            StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(prototype));
            context.buffer.Append("\"");
            context.buffer.Append(buffer);
            context.buffer.Append("\"");
            context.buffer.Append("\n");
        }

        if (!Repository::IsEmpty(rid))
        {
            context.Indent();
            context.buffer.Append("_object: {\n");
            context.AddIndentation();
            context.AddIndentation();
            WriteResource(context, rid);
            context.RemoveIndentation();
            context.RemoveIndentation();
            context.Indent();
            context.buffer.Append("}");
            context.buffer.Append("\n");
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

            if (type == ResourceFieldType::Value)
            {
                ConstPtr value = object.GetValue(i);
                if (!value) continue;

                context.Indent();
                context.buffer.Append(name);
                context.buffer.Append(": ");

                TypeHandler* handler = object.GetFieldType(i);
                TypeInfo typeInfo = handler->GetTypeInfo();
                WriteField(context, (VoidPtr) value, typeInfo, nullptr);
                context.buffer.Append("\n");
            }
            else if (type == ResourceFieldType::SubObject && object.Has(i))
            {
                RID subObject = object.GetSubObject(i);
                if (!subObject) continue;

                context.Indent();
                context.buffer.Append(name);
                context.buffer.Append(": ");
                context.buffer.Append("{\n");
                context.AddIndentation();
                WriteResourceInfo(context, subObject);
                context.RemoveIndentation();
                context.Indent();
                context.buffer.Append("}");
                context.buffer.Append("\n");
            }
            else if (type == ResourceFieldType::Stream && object.Has(i))
            {
                StreamObject* streamObject = object.GetStream(i);
                char strBuffer[17]{};
                usize bufSize = U64ToHex(streamObject->GetBufferId(), strBuffer);

                context.Indent();
                context.buffer.Append(name);
                context.buffer.Append(": ");
                context.buffer.Append("\"");
                context.buffer.Append(StringView{strBuffer, bufSize});
                context.buffer.Append("\"");
                context.buffer.Append("\n");
            }
            else if (type == ResourceFieldType::SubObjectSet)
            {
                u32 subObjectSetCount = object.GetSubObjectSetCount(i);
                u32 removedCount = object.GetRemoveFromPrototypeSubObjectSetCount(i);
                if (subObjectSetCount == 0 && removedCount == 0) continue;

                context.Indent();
                context.buffer.Append(name);
                context.buffer.Append(": ");
                context.buffer.Append("{\n");
                context.AddIndentation();

                if (removedCount > 0)
                {
                    context.Indent();
                    context.buffer.Append("_exclude");
                    context.buffer.Append(": ");
                    context.buffer.Append("[");
                    context.AddIndentation();

                    Array<RID> removedRids(removedCount);
                    object.GetRemoveFromPrototypeSubObjectSet(i, removedRids);

                    char buffer[StringConverter<UUID>::bufferCount + 1] = {};

                    for (RID removed: removedRids)
                    {
                        context.buffer.Append("\"");
                        StringConverter<UUID>::ToString(buffer, 0, Repository::GetUUID(removed));
                        context.buffer.Append(buffer);
                        context.buffer.Append("\",");
                    }
                    context.buffer.Erase(context.buffer.end() - 1, context.buffer.end());
                    context.RemoveIndentation();
                    context.buffer.Append("]\n");
                }

                if (subObjectSetCount > 0)
                {
                    Array<RID> subObjects(subObjectSetCount);
                    object.GetSubObjectSet(i, subObjects);

                    context.Indent();
                    context.buffer.Append("_values");
                    context.buffer.Append(": ");
                    context.buffer.Append("[");
                    context.AddIndentation();

                    for(RID subobject: subObjects)
                    {
                        context.buffer.Append("\n");
                        context.Indent();
                        context.buffer.Append("{\n");
                        context.AddIndentation();
                        WriteResourceInfo(context, subobject);
                        context.RemoveIndentation();
                        context.Indent();
                        context.buffer.Append("},");
                    }

                    context.buffer.Erase(context.buffer.end() - 1, context.buffer.end());
                    context.buffer.Append("\n");
                    context.RemoveIndentation();
                    context.Indent();
                    context.buffer.Append("]\n");
                }

                context.RemoveIndentation();
                context.Indent();
                context.buffer.Append("}");
                context.buffer.Append("\n");
            }
        }
        if (valueCount == 0)
        {
            TypeID resourceType = Repository::GetResourceTypeID(rid);
            TypeHandler* typeHandler = Registry::FindTypeById(resourceType);
            ConstPtr obj = Repository::Read(rid, resourceType);
            if (typeHandler && obj)
            {
                WriteObject(context, (VoidPtr)obj, typeHandler);
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