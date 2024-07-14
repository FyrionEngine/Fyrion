#include "Registry.hpp"
#include "String.hpp"
#include "SharedPtr.hpp"
#include "Array.hpp"
#include "HashMap.hpp"
#include "Logger.hpp"

namespace Fyrion
{
    namespace
    {
        HashMap<String, Array<SharedPtr<TypeHandler>>>      typesByName{};
        HashMap<TypeID, Array<SharedPtr<TypeHandler>>>      typesByID{};
        HashMap<String, SharedPtr<FunctionHandler>>         functionsByName{};
        HashMap<TypeID, Array<TypeHandler*>>                typesByAttribute{};
        HashMap<TypeID, Array<FunctionHandler*>>            functionsByAttribute{};
        EventHandler<OnTypeAdded>                           onTypeAddedEvent{};
        Logger&                                             logger = Logger::GetLogger("Fyrion::Registry");
    }

    ParamHandler::ParamHandler(usize index, const FieldInfo& fieldInfo) : m_fieldInfo(fieldInfo)
    {
        m_name.Append("param_");
        m_name.Append(index);
    }

    const FieldInfo& ParamHandler::GetFieldInfo() const
    {
        return m_fieldInfo;
    }

    const String& ParamHandler::GetName() const
    {
        return m_name;
    }

    void ParamHandler::SetName(const StringView& name)
    {
        m_name = name;
    }

    ValueHandler::ValueHandler(const String& valueDesc) : m_valueDesc(valueDesc)
    {
    }

    StringView ValueHandler::GetDesc() const
    {
        return m_valueDesc;
    }

    ConstPtr ValueHandler::GetValue() const
    {
        if (m_fnGetValue)
        {
            return m_fnGetValue(this);
        }
        return nullptr;
    }

    i64 ValueHandler::GetCode() const
    {
        if (m_fnGetCode)
        {
            return m_fnGetCode(this);
        }
        return I64_MIN;
    }

    bool ValueHandler::Compare(ConstPtr value) const
    {
        if (m_fnCompare)
        {
            return m_fnCompare(this, value);
        }
        return false;
    }

    ConstPtr AttributeHandler::GetAttribute(TypeID attributeId) const
    {
        if (auto it = m_attributes.Find(attributeId))
        {
            if (it->second->GetValue)
            {
                return it->second->GetValue(it->second.Get());
            }
        }
        return nullptr;
    }

    bool AttributeHandler::HasAttribute(TypeID attributeId) const
    {
        return m_attributes.Find(attributeId) != m_attributes.end();
    }

    Span<AttributeInfo*> AttributeHandler::GetAttributes() const
    {
        return m_attributeArray;
    }

    ConstructorHandler::ConstructorHandler(FieldInfo* params, usize paramsCount)
    {
        for (int i = 0; i < paramsCount; ++i)
        {
            m_params.EmplaceBack(i, params[i]);
        }
    }

    VoidPtr ConstructorHandler::NewInstance(Allocator& allocator, VoidPtr* params)
    {
        if (m_newInstanceFn)
        {
            return m_newInstanceFn(this, allocator, params);
        }
        return nullptr;
    }

    void ConstructorHandler::Construct(VoidPtr memory, VoidPtr* params)
    {
        if (m_placementNewFn)
        {
            m_placementNewFn(this, memory, params);
        }
    }

    FieldHandler::FieldHandler(const String& name, TypeHandler& owner) : name(name), owner(owner)
    {
        ownerCast = ForwardDerived;
    }

    StringView FieldHandler::GetName() const
    {
        return name;
    }

    FieldInfo FieldHandler::GetFieldInfo() const
    {
        if (fnGetFieldInfo)
        {
            return fnGetFieldInfo(this);
        }
        return {};
    }

    VoidPtr FieldHandler::GetFieldPointer(VoidPtr instance) const
    {
        if (fnGetFieldPointer)
        {
            return fnGetFieldPointer(this, instance);
        }
        return nullptr;
    }

    void FieldHandler::CopyValueTo(ConstPtr instance, VoidPtr value) const
    {
        if (fnCopyValueTo)
        {
            fnCopyValueTo(this, instance, value);
        }
    }

    void FieldHandler::SetValue(VoidPtr instance, ConstPtr value) const
    {
        if (fnSetValue)
        {
            fnSetValue(this, instance, value);
        }
    }

    FnCast FieldHandler::GetOwnerCaster() const
    {
        return ownerCast;
    }

    TypeHandler& FieldHandler::GetOwner() const
    {
        return owner;
    }

    void FieldHandler::Serialize(ArchiveWriter& writer, ConstPtr instance, ArchiveObject object) const
    {
        if (fnSerialize)
        {
            fnSerialize(this, instance, writer, object);
        }
        else if (TypeHandler* typeHandler = Registry::FindTypeById(GetFieldInfo().typeInfo.typeId))
        {
            writer.WriteValue(object, GetName(), Serialization::Serialize(typeHandler, writer, GetFieldPointer(const_cast<VoidPtr>(instance))));
        }
    }

    void FieldHandler::Deserialize(ArchiveReader& reader, VoidPtr instance, ArchiveObject object) const
    {
        if (fnDeserialize)
        {
            fnDeserialize(this, instance, reader, object);
        }
        else if (TypeHandler* typeHandler = Registry::FindTypeById(GetFieldInfo().typeInfo.typeId))
        {
            Serialization::Deserialize(typeHandler, reader, reader.ReadObject(object, GetName()), GetFieldPointer(instance));
        }
    }


    StringView FunctionHandler::GetName() const
    {
        return m_name;
    }

    StringView FunctionHandler::GetSimpleName() const
    {
        return m_simpleName;
    }

    TypeID FunctionHandler::GetFunctionId() const
    {
        return m_functionId;
    }

    Span<ParamHandler> FunctionHandler::GetParams() const
    {
        return m_params;
    }

    FieldInfo FunctionHandler::GetReturn() const
    {
        return m_return;
    }

    TypeHandler* FunctionHandler::GetOwner() const
    {
        return m_owner;
    }

    VoidPtr FunctionHandler::GetFunctionPointer() const
    {
        return m_functionPointer;
    }

    FunctionHandler::FnInvoke FunctionHandler::GetInvoker() const
    {
        return m_fnInvoke;
    }

    FnCast FunctionHandler::GetOwnerCaster() const
    {
        return m_ownerCast;
    }

    void FunctionHandler::Invoke(VoidPtr instance, VoidPtr ret, VoidPtr* params) const
    {
        if (m_fnInvoke)
        {
            m_fnInvoke(this, instance, ret, params);
        }
    }

    void FunctionHandler::OnAttributeCreated(TypeID attributeId)
    {
        auto fIt = functionsByAttribute.Find(attributeId);
        if (fIt == functionsByAttribute.end())
        {
            fIt = functionsByAttribute.Emplace(attributeId, Array<FunctionHandler*>{}).first;
        }
        fIt->second.EmplaceBack(this);
    }

    TypeHandler::TypeHandler(const StringView& name, const TypeInfo& typeInfo, u32 version) : name(name), typeInfo(typeInfo), version(version)
    {
        simpleName = Fyrion::GetSimpleName(name);
    }

    ConstructorHandler* TypeHandler::FindConstructor(TypeID* ids, usize size) const
    {
        u64 constructorId = size > 0 ? MurmurHash64(ids, size * sizeof(TypeID), HashSeed64) : 0;
        if (auto it = constructors.Find(constructorId))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    Span<ConstructorHandler*> TypeHandler::GetConstructors() const
    {
        return constructorArray;
    }

    FieldHandler* TypeHandler::FindField(const StringView& fieldName) const
    {
        if (auto it = fields.Find(fieldName))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    Span<FieldHandler*> TypeHandler::GetFields() const
    {
        return fieldArray;
    }

    FunctionHandler* TypeHandler::FindFunction(const StringView& functionName) const
    {
        if (auto it = functions.Find(functionName))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    Span<FunctionHandler*> TypeHandler::GetFunctions() const
    {
        return functionArray;
    }

    ValueHandler* TypeHandler::FindValueByName(const StringView& valueName) const
    {
        if(auto it = values.Find(valueName))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    ValueHandler* TypeHandler::FindValueByCode(i64 code) const
    {
        if(auto it = valuesByCode.Find(code))
        {
            return it->second;
        }
        return nullptr;
    }

    Span<ValueHandler*> TypeHandler::GetValues() const
    {
        return valuesArray;
    }

    Span<DerivedType> TypeHandler::GetDerivedTypes() const
    {
        return derivedTypes;
    }

    Array<TypeID> TypeHandler::GetBaseTypes() const
    {
        return baseTypesArray;
    }

    bool TypeHandler::IsDerivedFrom(TypeID typeId) const
    {
        if (baseTypes.Has(typeId))
        {
            return true;
        }

        for (auto& baseTypeId : baseTypesArray)
        {
            if (TypeHandler* typeHandler = Registry::FindTypeById(baseTypeId))
            {
                if (typeHandler->IsDerivedFrom(typeId))
                {
                    return true;
                }
            }
        }
        return false;
    }

    StringView TypeHandler::GetName() const
    {
        return name;
    }

    StringView TypeHandler::GetSimpleName() const
    {
        return simpleName;
    }

    const TypeInfo& TypeHandler::GetTypeInfo() const
    {
        return typeInfo;
    }

    u32 TypeHandler::GetVersion() const
    {
        return version;
    }

    FnCast TypeHandler::GetCaster(TypeID typeId) const
    {
        if (const auto it = baseTypes.Find(typeId))
        {
            return it->second;
        }
        return nullptr;
    }

    void TypeHandler::Destroy(VoidPtr instance, Allocator& allocator) const
    {
        if (fnDestroy)
        {
            fnDestroy(this, allocator, instance);
        }
    }

    void TypeHandler::Release(VoidPtr instance) const
    {
        if (fnRelease)
        {
            fnRelease(this, instance);
        }
    }

    void TypeHandler::Destructor(VoidPtr instance) const
    {
        if (fnDestructor)
        {
            fnDestructor(this, instance);
        }
    }

    void TypeHandler::Copy(ConstPtr source, VoidPtr dest) const
    {
        if (fnCopy)
        {
            fnCopy(this, source, dest);
        }
    }

    void TypeHandler::Move(VoidPtr source, VoidPtr dest) const
    {
        if (fnMove)
        {
            fnMove(this, source, dest);
        }
    }

    VoidPtr TypeHandler::Cast(TypeID typeId, VoidPtr instance) const
    {
        if (typeId == typeInfo.typeId)
        {
            return instance;
        }

        if (FnCast caster = GetCaster(typeId))
        {
            return caster(this, instance);
        }

        return nullptr;
    }

    void TypeHandler::OnAttributeCreated(TypeID attributeId)
    {
        auto fIt = typesByAttribute.Find(attributeId);
        if (fIt == typesByAttribute.end())
        {
            fIt = typesByAttribute.Emplace(attributeId, Array<TypeHandler*>{}).first;
        }
        fIt->second.EmplaceBack(this);
    }

    ///builders

    AttributeBuilder::AttributeBuilder(AttributeHandler& attributeHandler) : m_attributeHandler(attributeHandler)
    {
    }

    AttributeInfo& AttributeBuilder::NewAttribute(TypeID attributeId)
    {
        auto it = m_attributeHandler.m_attributes.Find(attributeId);
        if (it == m_attributeHandler.m_attributes.end())
        {
            it =m_attributeHandler.m_attributes.Emplace(attributeId, MakeShared<AttributeInfo>()).first;
            m_attributeHandler.m_attributeArray.EmplaceBack(it->second.Get());
            m_attributeHandler.OnAttributeCreated(attributeId);
        }
        return *it->second;
    }

    ValueBuilder::ValueBuilder(ValueHandler& valueHandler) : valueHandler(valueHandler)
    {
    }

    void ValueBuilder::SetFnGetValue(ValueHandler::FnGetValue fnGetValue)
    {
        valueHandler.m_fnGetValue = fnGetValue;
    }

    void ValueBuilder::SerFnGetCode(ValueHandler::FnGetCode fnGetCode)
    {
        valueHandler.m_fnGetCode = fnGetCode;
    }

    void ValueBuilder::SetFnCompare(ValueHandler::FnCompare fnCompare)
    {

        valueHandler.m_fnCompare = fnCompare;
    }

    ConstructorBuilder::ConstructorBuilder(ConstructorHandler& constructorHandler) : m_constructorHandler(constructorHandler)
    {
    }

    void ConstructorBuilder::SetPlacementNewFn(ConstructorHandler::PlacementNewFn placementNew)
    {
        m_constructorHandler.m_placementNewFn = placementNew;
    }

    void ConstructorBuilder::SetNewInstanceFn(ConstructorHandler::NewInstanceFn newInstance)
    {
        m_constructorHandler.m_newInstanceFn = newInstance;
    }

    FieldBuilder::FieldBuilder(FieldHandler& fieldHandler) : fieldHandler(fieldHandler)
    {
    }

    FieldHandler& FieldBuilder::GetFieldHandler() const
    {
        return fieldHandler;
    }

    void FieldBuilder::SetFnGetFieldInfo(FieldHandler::FnGetFieldInfo fnGetFieldInfo)
    {
        fieldHandler.fnGetFieldInfo = fnGetFieldInfo;
    }

    void FieldBuilder::SetFnGetFieldPointer(FieldHandler::FnGetFieldPointer fnGetFieldPointer)
    {
        fieldHandler.fnGetFieldPointer = fnGetFieldPointer;
    }

    void FieldBuilder::SetFnCopyValueTo(FieldHandler::FnCopyValueTo fnCopyValueTo)
    {
        fieldHandler.fnCopyValueTo = fnCopyValueTo;
    }

    void FieldBuilder::SetFnSetValue(FieldHandler::FnSetValue fnSetValue)
    {
        fieldHandler.fnSetValue = fnSetValue;
    }

    void FieldBuilder::SetFnSerialize(FieldHandler::FnSerialize fnSerialize)
    {
        fieldHandler.fnSerialize = fnSerialize;
    }

    void FieldBuilder::SetFnDeserialize(FieldHandler::FnDeserialize fnDeserialize)
    {
        fieldHandler.fnDeserialize = fnDeserialize;
    }

    void FieldBuilder::Copy(const FieldHandler& p_fieldHandler, TypeHandler& owner)
    {
        new(&fieldHandler) FieldHandler{p_fieldHandler};
        fieldHandler.ownerCast = owner.GetCaster(fieldHandler.owner.GetTypeInfo().typeId);
    }

    FunctionBuilder::FunctionBuilder(FunctionHandler& functionHandler) : m_functionHandler(functionHandler)
    {
    }

    void FunctionBuilder::Create(const FunctionHandlerCreation& creation)
    {
        m_functionHandler.m_params.Clear();
        m_functionHandler.m_name = creation.name;
        m_functionHandler.m_functionId = creation.functionId;
        m_functionHandler.m_return = creation.retInfo;
        m_functionHandler.m_owner =  Registry::FindTypeById(creation.owner);
        m_functionHandler.m_ownerCast = ForwardDerived;

        m_functionHandler.m_simpleName = GetSimpleName(m_functionHandler.m_name);

        for (int i = 0; i < creation.params.Size(); ++i)
        {
            m_functionHandler.m_params.EmplaceBack(i, creation.params[i]);
        }
    }

    void FunctionBuilder::Create(FunctionHandler& functionHandler, TypeHandler& owner)
    {
        m_functionHandler = functionHandler;
        m_functionHandler.m_owner = &owner;
        m_functionHandler.m_ownerCast = owner.GetCaster(functionHandler.m_owner->GetTypeInfo().typeId);
    }

    FunctionHandler& FunctionBuilder::GetFunctionHandler() const
    {
        return m_functionHandler;
    }

    ParamHandler& FunctionBuilder::GetParam(usize index)
    {
        return m_functionHandler.m_params[index];
    }

    void FunctionBuilder::SetFnInvoke(FunctionHandler::FnInvoke fnInvoke)
    {
        m_functionHandler.m_fnInvoke = fnInvoke;
    }

    void FunctionBuilder::SetFunctionPointer(VoidPtr functionPointer)
    {
        m_functionHandler.m_functionPointer = functionPointer;
    }

    TypeBuilder::TypeBuilder(TypeHandler& typeHandler) :  typeHandler(typeHandler)
    {

    }

    void TypeBuilder::SetFnDestroy(TypeHandler::FnDestroy fnDestroy)
    {
        typeHandler.fnDestroy = fnDestroy;
    }

    void TypeBuilder::SetFnCopy(TypeHandler::FnCopy fnCopy)
    {
        typeHandler.fnCopy = fnCopy;
    }

    void TypeBuilder::SetFnDestructor(TypeHandler::FnDestructor destructor)
    {
        typeHandler.fnDestructor = destructor;
    }

    void TypeBuilder::SetFnRelease(TypeHandler::FnRelease fnRelease)
    {
        typeHandler.fnRelease = fnRelease;
    }

    void TypeBuilder::SetFnMove(TypeHandler::FnMove fnMove)
    {
        typeHandler.fnMove = fnMove;
    }

    ConstructorBuilder TypeBuilder::NewConstructor(TypeID* ids, FieldInfo* params, usize size)
    {
        u64 constructorId = size > 0 ? MurmurHash64(ids, size * sizeof(TypeID), HashSeed64) : 0;
        auto it = typeHandler.constructors.Find(constructorId);

        if (it == typeHandler.constructors.end())
        {
            it = typeHandler.constructors.Emplace(constructorId, MakeShared<ConstructorHandler>(params, size)).first;
            typeHandler.constructorArray.EmplaceBack(it->second.Get());
        }
        return *it->second;
    }

    FieldBuilder TypeBuilder::NewField(const StringView& fieldName)
    {
        auto it = typeHandler.fields.Find(fieldName);
        if (it == typeHandler.fields.end())
        {
            it = typeHandler.fields.Emplace(fieldName, MakeShared<FieldHandler>(fieldName, typeHandler)).first;
            typeHandler.fieldArray.EmplaceBack(it->second.Get());
        }
        return FieldBuilder{*it->second};
    }

    ValueBuilder TypeBuilder::NewValue(const StringView& valueDesc, i64 code)
    {
        auto it = typeHandler.values.Find(valueDesc);
        if (it == typeHandler.values.end())
        {
            it = typeHandler.values.Emplace(valueDesc, MakeShared<ValueHandler>(valueDesc)).first;
            typeHandler.valuesArray.EmplaceBack(it->second.Get());
            typeHandler.valuesByCode.Emplace(code, it->second.Get());
        }
        return ValueBuilder{*it->second};
    }

    FunctionBuilder TypeBuilder::NewFunction(const FunctionHandlerCreation& creation)
    {
        FunctionBuilder builder = NewFunction(creation.name);
        builder.Create(creation);
        return builder;
    }

    FunctionBuilder TypeBuilder::NewFunction(StringView functionName)
    {
        auto it = typeHandler.functions.Find(functionName);
        if (it == typeHandler.functions.end())
        {
            it = typeHandler.functions.Emplace(String{functionName}, MakeShared<FunctionHandler>()).first;
            typeHandler.functionArray.EmplaceBack(it->second.Get());
        }

        return FunctionBuilder{*it->second};
    }

    void TypeBuilder::AddBaseType(TypeID typeId, FnCast fnCast)
    {
        TypeHandler* baseType = Registry::FindTypeById(typeId);
        FY_ASSERT(baseType, "Base Type not found");
        if (baseType)
        {
            baseType->derivedTypes.EmplaceBack(typeHandler.GetTypeInfo().typeId, fnCast);
            typeHandler.baseTypes.Insert(typeId, fnCast);
            typeHandler.baseTypesArray.EmplaceBack(typeId);

            for(const auto& it : baseType->functions)
            {
                FunctionBuilder functionBuilder = NewFunction(it.first);
                functionBuilder.Create(*it.second, typeHandler);
            }

            for (const auto& it : baseType->fields)
            {
                FieldBuilder builder = NewField(it.first);
                builder.Copy(*it.second, typeHandler);
            }
        }
    }

    void TypeBuilder::Build() const
    {
        onTypeAddedEvent.Invoke(typeHandler);
    }

    TypeHandler& TypeBuilder::GetTypeHandler() const
    {
        return typeHandler;
    }


    TypeBuilder Registry::NewType(const StringView& name, const TypeInfo& typeInfo)
    {
        auto itByName = typesByName.Find(name);
        if (!itByName)
        {
            itByName = typesByName.Emplace(name, Array<SharedPtr<TypeHandler>>{}).first;
        }

        auto itById = typesByID.Find(typeInfo.typeId);
        if (!itById)
        {
            itById = typesByID.Emplace(typeInfo.typeId, Array<SharedPtr<TypeHandler>>{}).first;
        }

        usize version = itByName->second.Size() + 1;
        SharedPtr<TypeHandler> typeHandler = MakeShared<TypeHandler>(name, typeInfo, version);

        itByName->second.EmplaceBack(typeHandler);
        itById->second.EmplaceBack(typeHandler);

        logger.Debug("Type {} Registered ", name, version);

        return {*typeHandler};
    }

    FunctionBuilder Registry::NewFunction(const FunctionHandlerCreation& creation)
    {
        auto it = functionsByName.Find(creation.name);
        if (it == functionsByName.end())
        {
            it = functionsByName.Emplace(String{creation.name}, MakeShared<FunctionHandler>()).first;
        }

        FunctionBuilder builder{*it->second};
        builder.Create(creation);
        return builder;
    }

    FunctionHandler* Registry::FindFunctionByName(const StringView& name)
    {
        if (auto it = functionsByName.Find(name))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    Span<FunctionHandler*> Registry::FindFunctionsByAttribute(TypeID typeId)
    {
        auto it = functionsByAttribute.Find(typeId);
        if (it != functionsByAttribute.end())
        {
            return it->second;
        }
        return {};
    }

    TypeHandler* Registry::FindTypeByName(const StringView& name)
    {
        if (auto it = typesByName.Find(name))
        {
            return it->second.Back().Get();
        }
        return nullptr;
    }

    TypeHandler* Registry::FindTypeById(TypeID typeId)
    {
        if (auto it = typesByID.Find(typeId))
        {
            return it->second.Back().Get();
        }
        return nullptr;
    }

    Span<TypeHandler*> Registry::FindTypesByAttribute(TypeID typeId)
    {
        if (auto it = typesByAttribute.Find(typeId))
        {
            return it->second;
        }
        return {};
    }

    void RegistryShutdown()
    {
        typesByName.Clear();
        typesByID.Clear();
        functionsByName.Clear();
        typesByAttribute.Clear();
        functionsByAttribute.Clear();
    }
}