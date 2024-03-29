#pragma once

#include "Fyrion/Common.hpp"
#include "Allocator.hpp"
#include "StringView.hpp"
#include "TypeInfo.hpp"
#include "String.hpp"
#include "SharedPtr.hpp"
#include "HashMap.hpp"
#include "Span.hpp"

namespace Fyrion
{

    class TypeBuilder;
    struct FieldInfo;

    struct FunctionInfo
    {
        TypeID functionId{};
        TypeID ownerId{};
        usize paramCount{};
        const FieldInfo* paramsInfo{};
        const FieldInfo* returnInfo{};
    };

    struct FieldInfo
    {
        TypeID ownerId{};
        bool isConst{};
        bool isPointer{};
        bool isReference{};
        TypeInfo typeInfo{};
    };

    template<typename Owner, typename Field>
    constexpr FieldInfo MakeFieldInfo()
    {
        return FieldInfo{
            .ownerId = GetTypeID<Owner>(),
            .isConst = false,
            .isPointer  = false,
            .isReference  = false,
            .typeInfo = GetTypeInfo<Field>()
        };
    }

    struct FunctionHandlerCreation
    {
        StringView            name{};
        TypeID                functionId{};
        TypeID                owner{};
        Span<const FieldInfo> params{};
        FieldInfo             retInfo{};
    };

    struct AttributeInfo
    {
        typedef ConstPtr(*FnGetValue)(const AttributeInfo* handler);
        typedef TypeInfo(*FnGetTypeInfo)(const AttributeInfo* handler);

        VoidPtr         userData{};

        FnGetValue      GetValue{};
        FnGetTypeInfo   GetInfo{};
    };

    class FY_API AttributeHandler
    {
    private:
        HashMap<TypeID, SharedPtr<AttributeInfo>>  m_attributes{};
        Array<AttributeInfo*>                      m_attributeArray{};
    public:
        ConstPtr                GetAttribute(TypeID attributeId) const;
        bool                    HasAttribute(TypeID attributeId) const;
        Span<AttributeInfo*>    GetAttributes() const;

        template<typename AttType>
        const AttType* GetAttribute() const
        {
            return static_cast<const AttType*>(GetAttribute(GetTypeID<AttType>()));
        }

        template<typename AttType>
        bool HasAttribute() const
        {
            return HasAttribute(GetTypeID<AttType>());
        }
    };


    class FY_API ConstructorHandler : public AttributeHandler
    {
    public:
        VoidPtr     NewInstance(Allocator& allocator, VoidPtr* params);
        void        Construct(VoidPtr memory, VoidPtr* params);
    };

    class FY_API FieldHandler : public AttributeHandler
    {
    public:
        explicit FieldHandler(const String& name);

        StringView  GetName() const;
        FieldInfo   GetFieldInfo() const;
        VoidPtr     GetFieldPointer(VoidPtr instance) const;
        void        CopyValueTo(ConstPtr instance, VoidPtr value);
        void        SetValue(VoidPtr instance, ConstPtr value);

        template<typename T>
        T& GetFieldAs(VoidPtr instance)
        {
            return *static_cast<T*>(GetFieldPointer(instance));
        }

        template<typename T>
        void SetValueAs(VoidPtr instance, const T& value)
        {
            SetValue(instance, &value);
        }
    };

    class FY_API ParamHandler
    {
    public:
    };


    class FY_API FunctionHandler : public AttributeHandler
    {
    public:
        typedef void(*FnCall)(const FunctionHandler* handler, VoidPtr instance, VoidPtr ret, VoidPtr* params);
    private:
        String              m_name{};
        String              m_simpleName{};
        TypeID              m_functionId{U64_MAX};
        TypeID              m_owner{};
        Array<ParamHandler> m_params{};
        FieldInfo           m_return{};
        FnCall              m_fnCall{};
        VoidPtr             m_functionPointer{};

        HashMap<TypeID, SharedPtr<AttributeHandler>> m_Attributes;
        Array<AttributeHandler*>                     m_AttributeArray;
    public:
        StringView          GetName() const;
        Span<ParamHandler>  GetParams() const;
        FieldInfo           GetReturn() const;
        TypeID              GetOwner() const;
        VoidPtr             GetFunctionPointer() const;


        void Invoke(VoidPtr instance, VoidPtr ret, VoidPtr* params) const;
    };

    class FY_API TypeHandler : public AttributeHandler
    {
    public:
        typedef void (*FnDestroy)(const TypeHandler* typeHandler, Allocator& allocator, VoidPtr instance);
        typedef void (*FnDestructor)(const TypeHandler* typeHandler, VoidPtr instance);
        typedef void (*FnCopy)(const TypeHandler* typeHandler, ConstPtr source, VoidPtr dest);
        typedef void (*FnMove)(const TypeHandler* typeHandler, VoidPtr source, VoidPtr dest);

        friend class TypeBuilder;
    private:
        String       m_name{};
        TypeInfo     m_typeInfo{};
        u32          m_version{};
        FnDestroy    m_fnDestroy{};
        FnCopy       m_fnCopy{};
        FnDestructor m_fnDestructor{};
        FnMove       m_fnMove{};

        HashMap<usize, SharedPtr<ConstructorHandler>> m_constructors{};
        Array<ConstructorHandler*>                    m_constructorArray{};
        HashMap<String, SharedPtr<FieldHandler>>      m_fields{};
        Array<FieldHandler*>                          m_fieldArray{};
        HashMap<String, SharedPtr<FunctionHandler>>   m_functions{};
        Array<FunctionHandler*>                       m_functionArray{};
    public:
        TypeHandler(const StringView& name, const TypeInfo& typeInfo, u32 version);

        ConstructorHandler*             FindConstructor(TypeID* ids, usize size) const;
        Span<ConstructorHandler*>       GetConstructors() const;

        FieldHandler*                   FindField(const StringView& fieldName) const;
        Span<FieldHandler*>             GetFields() const;

        FunctionHandler*                FindFunction(const StringView& functionName) const;
        Span<FunctionHandler*>          GetFunctions() const;


        StringView          GetName() const;
        const TypeInfo&     GetTypeInfo() const;
        u32                 GetVersion() const;

        void                Destroy(VoidPtr instance, Allocator& allocator = MemoryGlobals::GetDefaultAllocator()) const;
        void                Destructor(VoidPtr instance) const;
        void                Copy(ConstPtr source, VoidPtr dest) const;
        void                Move(VoidPtr source, VoidPtr dest) const;

        VoidPtr NewInstance(Allocator& allocator = MemoryGlobals::GetDefaultAllocator()) const
        {
            if (ConstructorHandler* constructor = FindConstructor(nullptr, 0))
            {
                return constructor->NewInstance(allocator, nullptr);
            }
            return nullptr;
        }

        template<typename ...Args>
        VoidPtr NewInstance(Allocator& allocator, Args&& ...args) const
        {
            TypeID ids[] = {GetTypeID<Args>()...,};

            if (ConstructorHandler* constructor = FindConstructor(ids, sizeof...(args)))
            {
                VoidPtr params[] = {&args...};
                return constructor->NewInstance(allocator, params);
            }
            return nullptr;
        }

        template<typename ...Args>
        VoidPtr NewInstance(Args&& ...args) const
        {
            return NewInstance(MemoryGlobals::GetDefaultAllocator(), Traits::Forward<Args>(args)...);
        }
    };

    ///-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    ///-------------------------------------------------------------------------------------Builders----------------------------------------------------------------------------------------------------
    ///-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    class FY_API AttributeBuilder
    {
    private:
        AttributeHandler& m_attributeHandler;
    public:
        explicit AttributeBuilder(AttributeHandler& attributeHandler);
    };


    class FY_API ConstructorBuilder
    {
    private:
        ConstructorHandler& m_constructorHandler;
    public:
        ConstructorBuilder(ConstructorHandler& constructorHandler);
    };

    class FY_API FieldBuilder
    {
    private:
        FieldHandler& m_fieldHandler;
    public:
        explicit FieldBuilder(FieldHandler& fieldHandler);
    };

    class FY_API FunctionBuilder
    {
    private:
        FunctionHandler& m_functionHandler;
    public:
        explicit FunctionBuilder(FunctionHandler& functionHandler);
    };

    class FY_API TypeBuilder
    {
    private:
        TypeHandler& m_typeHandler;
    public:
        TypeBuilder(TypeHandler& typeHandler);

        void                    SetFnDestroy(TypeHandler::FnDestroy fnDestroy);
        void                    SetFnCopy(TypeHandler::FnCopy fnCopy);
        void                    SetFnDestructor(TypeHandler::FnDestructor destructor);
        void                    SetFnMove(TypeHandler::FnMove fnMove);

        ConstructorBuilder      NewConstructor(TypeID* ids, FieldInfo* params, usize size);
        FieldBuilder            NewField(const StringView& fieldName);
        FunctionBuilder         NewFunction(const FunctionHandlerCreation& creation);
    };

    ///-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    ///------------------------------------------------------------------------------Native Handlers----------------------------------------------------------------------------------------------------
    ///-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


    template<typename Owner>
    class NativeAttributeBuilder
    {
    public:
        template<typename TypeAttr, typename ...AttrArgs>
        inline Owner& Attribute(AttrArgs&& ... args)
        {
            //NativeAttributeHandlerBuild<decltype(*this), TypeAttr>::Build(m_FieldHandler.NewAttribute(GetTypeID<TypeAttr>()), Traits::Forward<AttrArgs>(args)...);
            return *static_cast<Owner*>(this);
        }
    };


    template<typename Owner, typename ...Args>
    class NativeConstructorHandler
    {
    public:
        explicit NativeConstructorHandler(ConstructorBuilder constructorBuilder) : m_constructorHandler(constructorBuilder)
        {
//            constructorHandler.SetNewInstanceFn(&NewInstanceImpl);
//            constructorHandler.SetPlacementNewFn(PlacementNewImpl);
        }

    private:
        ConstructorBuilder m_constructorHandler;

        template<typename ...Vals>
        static void Eval(VoidPtr memory, VoidPtr* params, Traits::IndexSequence<>, Vals&& ...vals)
        {
            if constexpr (Traits::IsAggregate<Owner>)
            {
                new(PlaceHolder(), memory) Owner{Traits::Forward<Vals>(vals)...};
            }
            else
            {
                new(PlaceHolder(), memory) Owner(Traits::Forward<Vals>(vals)...);
            }
        }

        template<typename T, typename ...Tp, usize I, usize... Is, typename ...Vls>
        static void Eval(VoidPtr memory, VoidPtr* params, Traits::IndexSequence<I, Is...> seq, Vls&& ...vls)
        {
            return Eval<Tp...>(memory, params, Traits::IndexSequence<Is...>(), Traits::Forward<Vls>(vls)..., *static_cast<T*>(params[I]));
        }

        static void PlacementNewImpl(ConstructorHandler* handler, VoidPtr memory, VoidPtr* params)
        {
            const usize size = sizeof...(Args);
            Eval<Args...>(memory, params, Traits::MakeIntegerSequence<usize, size>{});
        }

        static VoidPtr NewInstanceImpl(ConstructorHandler* handler, Allocator& allocator, VoidPtr* params)
        {
            VoidPtr ptr = allocator.MemAlloc(sizeof(Owner), alignof(Owner));
            PlacementNewImpl(handler, ptr, params);
            return ptr;
        }
    };


    //fields

    template<auto mfp, typename Owner, typename Field>
    class NativeFieldHandler
    {
    private:
        FieldBuilder m_fieldBuilder;
    public:
        explicit NativeFieldHandler(FieldBuilder fieldBuilder) : m_fieldBuilder(fieldBuilder)
        {

        }
    };

    template<auto mfp, typename Field>
    struct FieldTemplateDecomposer
    {
    };

    template<auto mfp, typename Owner, typename FieldType>
    struct FieldTemplateDecomposer<mfp, FieldType Owner::*>
    {

        static TypeID GetTypeId()
        {
            return GetTypeID<FieldType>();
        }

        static auto CreateHandler(FieldBuilder fieldBuilder)
        {
            return NativeFieldHandler<mfp, Owner, FieldType>(fieldBuilder);
        }
    };


    //functions

    template<auto fp, typename Return, typename ...Args>
    class NativeFunctionHandler
    {
    private:
        FunctionBuilder m_functionBuilder;
    public:
        NativeFunctionHandler(const FunctionBuilder& functionBuilder) : m_functionBuilder(functionBuilder)
        {

        }
    };

    template<auto mfp, typename Return, typename Owner, typename ...Args>
    class NativeMemberFunctionHandler : public NativeAttributeBuilder<NativeMemberFunctionHandler<mfp, Return, Owner, Args...>>
    {
    private:
        FunctionBuilder m_functionBuilder;
    public:
        explicit NativeMemberFunctionHandler(const FunctionBuilder& functionBuilder) : m_functionBuilder(functionBuilder)
        {
        }
    };

    template<auto func, typename Function>
    struct MemberFunctionTemplateDecomposer
    {
    };

    template<auto mfp, typename Return, typename Owner>
    struct MemberFunctionTemplateDecomposer<mfp, Return(Owner::*)()>
    {
        using FuncType = MemberFunctionTemplateDecomposer<mfp, Return(Owner::*)()>;

        static decltype(auto) CreateHandler(FunctionBuilder functionBuilder)
        {
            return NativeMemberFunctionHandler<mfp, Return, Owner>(functionBuilder);
        }

        static FunctionHandlerCreation MakeCreation(const StringView& name)
        {
            return FunctionHandlerCreation{
                .name = name,
                .functionId = GetTypeID<FuncType>(),
                .owner = GetTypeID<Owner>(),
                .retInfo = MakeFieldInfo<Owner, Return>()
            };
        }
    };

    template<auto mfp, typename Return, typename Owner, typename ...Args>
    struct MemberFunctionTemplateDecomposer<mfp, Return(Owner::*)(Args...)>
    {
        using FuncType = MemberFunctionTemplateDecomposer<mfp, Return(Owner::*)()>;
        static constexpr FieldInfo Params[] = {MakeFieldInfo<Owner, Args>()...};

        static decltype(auto) CreateHandler(FunctionBuilder functionBuilder)
        {
            return NativeMemberFunctionHandler<mfp, Return, Owner, Args...>(functionBuilder);
        }

        static FunctionHandlerCreation MakeCreation(const StringView& name)
        {
            return FunctionHandlerCreation{
                .name = name,
                .functionId = GetTypeID<FuncType>(),
                .owner = GetTypeID<Owner>(),
                .params = {Params, sizeof...(Args)},
                .retInfo = MakeFieldInfo<Owner, Return>()
            };
        }
    };

    template<auto fp, typename Return>
    struct MemberFunctionTemplateDecomposer<fp, Return(*)()>
    {
        using FuncType = MemberFunctionTemplateDecomposer<fp, Return(*)()>;

        static decltype(auto) CreateHandler(FunctionBuilder functionBuilder)
        {
            return NativeFunctionHandler<fp, Return>(functionBuilder);
        }

        static FunctionHandlerCreation MakeCreation(const StringView& name)
        {
            return FunctionHandlerCreation{
                .name = name,
                .functionId = GetTypeID<FuncType>(),
                .retInfo = MakeFieldInfo<void, Return>()
            };
        }
    };

    template<auto fp, typename Return, typename ...Args>
    struct MemberFunctionTemplateDecomposer<fp, Return(*)(Args...)>
    {
        using FuncType = MemberFunctionTemplateDecomposer<fp, Return(*)(Args...)>;
        static constexpr FieldInfo Params[] = {MakeFieldInfo<void, Args>()...};

        static decltype(auto) CreateHandler(FunctionBuilder functionBuilder)
        {
            return NativeFunctionHandler<fp, Return, Args...>(functionBuilder);
        }

        static FunctionHandlerCreation MakeCreation(const StringView& name)
        {
            return FunctionHandlerCreation{
                .name = name,
                .functionId = GetTypeID<FuncType>(),
                .params = {Params, sizeof...(Args)},
                .retInfo = MakeFieldInfo<void, Return>()
            };
        }
    };


    //type

    template<typename Type>
    class NativeTypeHandler : public NativeAttributeBuilder<NativeTypeHandler<Type>>
    {
    private:
        TypeBuilder m_typeHandlerBuilder;
    public:
        explicit NativeTypeHandler(TypeBuilder typeHandlerBuilder) : m_typeHandlerBuilder(typeHandlerBuilder)
        {

        }

        inline auto Constructor()
        {
            return NativeConstructorHandler<Type>(m_typeHandlerBuilder.NewConstructor(nullptr, nullptr, 0));
        }

        template<typename ...Args>
        inline auto Constructor()
        {
            FieldInfo params[] = {MakeFieldInfo<void, Args>()...};
            TypeID ids[] = {GetTypeID<Args>()...,};
            return NativeConstructorHandler<Type, Args...>(m_typeHandlerBuilder.NewConstructor(ids, params, sizeof...(Args)));
        }

        template<auto mfp>
        inline auto Field(const StringView& name)
        {
            using FieldDecomp = FieldTemplateDecomposer<mfp, decltype(mfp)>;
            return FieldDecomp::CreateHandler(m_typeHandlerBuilder.NewField(name));
        }

        template<auto mfp>
        inline auto Function(const StringView& name)
        {
            using FuncType = Traits::RemoveConstFunc<decltype(mfp)>;
            using DecompType = MemberFunctionTemplateDecomposer<mfp, FuncType>;
            return DecompType::CreateHandler(m_typeHandlerBuilder.NewFunction(DecompType::MakeCreation(name)));
        }
    };

    namespace Registry
    {
        FY_API TypeBuilder      NewType(const StringView& name, const TypeInfo& typeInfo);
        FY_API TypeHandler*     FindTypeByName(const StringView& name);
        FY_API TypeHandler*     FindTypeById(TypeID typeId);

        template<typename T>
        inline decltype(auto) Type()
        {
            return NativeTypeHandler<T>(NewType(GetTypeName<T>(), GetTypeInfo<T>()));
        }

        template<typename T>
        inline decltype(auto) Type(const StringView& name)
        {
            return NativeTypeHandler<T>(NewType(name, GetTypeInfo<T>()));
        }

        template<typename T>
        TypeHandler* FindType()
        {
            return FindTypeById(GetTypeID<T>());
        }
    }
}