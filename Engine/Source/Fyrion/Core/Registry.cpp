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
        Logger&                                             logger = Logger::GetLogger("Fyrion::Reflection", LogLevel_Debug);
    }

    ConstPtr AttributeHandler::GetAttribute(TypeID attributeId) const
    {
        return nullptr;
    }

    bool AttributeHandler::HasAttribute(TypeID attributeId) const
    {
        return false;
    }

    Span<AttributeInfo*> AttributeHandler::GetAttributes() const
    {
        return Span<AttributeInfo*>();
    }

    VoidPtr ConstructorHandler::NewInstance(Allocator& allocator, VoidPtr* params)
    {
        return nullptr;
    }

    void ConstructorHandler::Construct(VoidPtr memory, VoidPtr* params)
    {

    }

    FieldHandler::FieldHandler(const String& name)
    {

    }

    StringView FieldHandler::GetName() const
    {
        return Fyrion::StringView();
    }

    FieldInfo FieldHandler::GetFieldInfo() const
    {
        return FieldInfo();
    }

    VoidPtr FieldHandler::GetFieldPointer(VoidPtr instance) const
    {
        return nullptr;
    }

    void FieldHandler::CopyValueTo(ConstPtr instance, VoidPtr value)
    {

    }

    void FieldHandler::SetValue(VoidPtr instance, ConstPtr value)
    {

    }

    StringView FunctionHandler::GetName() const
    {
        return Fyrion::StringView();
    }

    Span<ParamHandler> FunctionHandler::GetParams() const
    {
        return Span<ParamHandler>();
    }

    FieldInfo FunctionHandler::GetReturn() const
    {
        return FieldInfo();
    }

    TypeID FunctionHandler::GetOwner() const
    {
        return 0;
    }

    VoidPtr FunctionHandler::GetFunctionPointer() const
    {
        return nullptr;
    }

    void FunctionHandler::Invoke(VoidPtr instance, VoidPtr ret, VoidPtr* params) const
    {

    }

    TypeHandler::TypeHandler(const StringView& name, const TypeInfo& typeInfo, u32 version)
    {

    }

    ConstructorHandler* TypeHandler::FindConstructor(TypeID* ids, usize size) const
    {
        return nullptr;
    }

    Span<ConstructorHandler*> TypeHandler::GetConstructors() const
    {
        return Span<ConstructorHandler*>();
    }

    FieldHandler* TypeHandler::FindField(const StringView& fieldName) const
    {
        return nullptr;
    }

    Span<FieldHandler*> TypeHandler::GetFields() const
    {
        return Span<FieldHandler*>();
    }

    FunctionHandler* TypeHandler::FindFunction(const StringView& functionName) const
    {
        return nullptr;
    }

    Span<FunctionHandler*> TypeHandler::GetFunctions() const
    {
        return Span<FunctionHandler*>();
    }

    StringView TypeHandler::GetName() const
    {
        return Fyrion::StringView();
    }

    const TypeInfo& TypeHandler::GetTypeInfo() const
    {
        return <#initializer#>;
    }

    u32 TypeHandler::GetVersion() const
    {
        return 0;
    }

    void TypeHandler::Destroy(VoidPtr instance, Allocator& allocator) const
    {

    }

    void TypeHandler::Destructor(VoidPtr instance) const
    {

    }

    void TypeHandler::Copy(ConstPtr source, VoidPtr dest) const
    {

    }

    void TypeHandler::Move(VoidPtr source, VoidPtr dest) const
    {

    }

    ///builders

    AttributeBuilder::AttributeBuilder(AttributeHandler& attributeHandler) : m_attributeHandler(attributeHandler)
    {}

    ConstructorBuilder::ConstructorBuilder(ConstructorHandler& constructorHandler) : m_constructorHandler(constructorHandler)
    {
    }

    FieldBuilder::FieldBuilder(FieldHandler& fieldHandler) : m_fieldHandler(fieldHandler)
    {}

    FunctionBuilder::FunctionBuilder(FunctionHandler& functionHandler) : m_functionHandler(functionHandler)
    {}

    TypeBuilder::TypeBuilder(TypeHandler& typeHandler) :  m_typeHandler(typeHandler)
    {

    }

    void TypeBuilder::SetFnDestroy(TypeHandler::FnDestroy fnDestroy)
    {

    }

    void TypeBuilder::SetFnCopy(TypeHandler::FnCopy fnCopy)
    {

    }

    void TypeBuilder::SetFnDestructor(TypeHandler::FnDestructor destructor)
    {

    }

    void TypeBuilder::SetFnMove(TypeHandler::FnMove fnMove)
    {

    }

    ConstructorHandler& TypeBuilder::NewConstructor(TypeID* ids, FieldInfo* params, usize size)
    {
        return <#initializer#>;
    }

    FieldHandler& TypeBuilder::NewField(const StringView& fieldName)
    {
        return <#initializer#>;
    }

    FunctionHandler& TypeBuilder::NewFunction(const FunctionHandlerCreation& creation)
    {
        return <#initializer#>;
    }



    TypeHandler& Registry::NewType(const StringView& name, const TypeInfo& typeInfo)
    {

    }

    TypeHandler* Registry::FindTypeByName(const StringView& name)
    {
        return nullptr;
    }

    TypeHandler* Registry::FindTypeById(TypeID typeId)
    {
        return nullptr;
    }
}