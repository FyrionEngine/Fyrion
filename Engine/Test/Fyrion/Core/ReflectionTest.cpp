#include "doctest.h"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Engine.hpp"

using namespace Fyrion;

namespace
{
	struct IncompleteType;

	struct ReflectionTestStruct
	{
		u32    uint;
		i32    iint;
		String string;
	};

	struct TestAttribute
	{
		i32 value{};
	};

	struct IncompleteAttr;

	struct OtherTestAttribute
	{
		String value{};
	};

	class ReflectionTestClass
	{
	public:

		static i32 constructorCalls;
		static i32 destructorCalls;

		ReflectionTestClass() : m_int(10), m_string("Empty")
		{
            constructorCalls++;
		}

		ReflectionTestClass(i32 anInt, const String& string) : m_int(anInt), m_string(string)
		{
            constructorCalls++;
		}

		virtual ~ReflectionTestClass()
		{
            destructorCalls++;
		}

		i32 GetInt() const
		{
			return m_int;
		}

		const String& GetString() const
		{
			return m_string;
		}

		static void ResetCount()
		{
            constructorCalls = 0;
            destructorCalls  = 0;
		}

	private:
		i32    m_int;
		String m_string;
	};

	struct ReflectionFunctions
	{
		static i32 staticVoidFuncCalls;
		i32        calls = 0;

		void VoidFunc()
		{
            calls++;
		}

		i32 ParamsRetFunc(i32 a, i32 b)
		{
            calls++;
			return a + b;
		}

		static i32 StaticFunc(i32 a, i32 b)
		{
            staticVoidFuncCalls++;
			return a * b;
		}

		static void StaticFuncNoParam()
		{
            staticVoidFuncCalls++;
		}

	};

	i32 ReflectionTestClass::constructorCalls    = 0;
	i32 ReflectionTestClass::destructorCalls     = 0;
	i32 ReflectionFunctions::staticVoidFuncCalls = 0;

	void TestTypeRegister()
	{
		auto testStruct = Registry::Type<ReflectionTestStruct>();
		testStruct.Field<&ReflectionTestStruct::uint>("uint");
		testStruct.Field<&ReflectionTestStruct::iint>("iint");
		testStruct.Field<&ReflectionTestStruct::string>("string");
	}


	TEST_CASE("Core::ReflectionBasics")
	{
		Registry::Type<IncompleteType>().Attribute<TestAttribute>(20);

		auto typeClass = Registry::Type<ReflectionTestClass>();
		typeClass.Constructor<i32, String>();
		typeClass.Attribute<TestAttribute>(10);
		typeClass.Attribute<IncompleteAttr>();


		TypeHandler* incompleteType = Registry::FindTypeByName("Tests::IncompleteType");
		CHECK(incompleteType->GetAttribute<TestAttribute>()->value == 20);
		CHECK(incompleteType->GetTypeInfo().size == 0);

		TypeHandler* testClass = Registry::FindTypeByName("Tests::ReflectionTestClass");
		REQUIRE(testClass != nullptr);

		CHECK(testClass->GetName() == "Tests::ReflectionTestClass");
		CHECK(testClass->GetTypeInfo().typeId == GetTypeID<ReflectionTestClass>());
		CHECK(testClass->GetTypeInfo().size == sizeof(ReflectionTestClass));

		CHECK(testClass->GetAttribute<TestAttribute>()->value == 10);

		{
			VoidPtr instance = testClass->NewInstance(123, String{"TestStr"});
			CHECK(ReflectionTestClass::constructorCalls == 1);
			REQUIRE(instance != nullptr);

			ReflectionTestClass& test = *static_cast<ReflectionTestClass*>(instance);
			CHECK(test.GetInt() == 123);
			CHECK(test.GetString() == "TestStr");

			testClass->Destroy(instance);
			CHECK(ReflectionTestClass::destructorCalls == 1);

			ReflectionTestClass::ResetCount();
		}

		{
            VoidPtr instance = testClass->NewInstance();
			CHECK(ReflectionTestClass::constructorCalls == 1);
			REQUIRE(instance != nullptr);

			ReflectionTestClass& test = *static_cast<ReflectionTestClass*>(instance);
			CHECK(test.GetInt() == 10);
			CHECK(test.GetString() == "Empty");

			testClass->Destroy(instance);
			CHECK(ReflectionTestClass::destructorCalls == 1);
			ReflectionTestClass::ResetCount();
		}

		TypeHandler* nullRet = Registry::FindTypeByName("ClassNameThatDontExists");
		CHECK(nullRet == nullptr);

		Engine::Destroy();
	}

	TEST_CASE("Core::ReflectionFields")
	{
		Engine::Init();

		TestTypeRegister();
		TypeHandler* testStruct = Registry::FindTypeByName("Tests::ReflectionTestStruct");
		REQUIRE(testStruct != nullptr);

		REQUIRE(testStruct->GetConstructors().Size() == 1);

		FieldHandler* uintField = testStruct->FindField("uint");
		FieldHandler* iintField = testStruct->FindField("iint");
		FieldHandler* stringField = testStruct->FindField("string");

		REQUIRE(uintField != nullptr);
		REQUIRE(iintField != nullptr);
		REQUIRE(stringField != nullptr);

		Span<FieldHandler*> fields = testStruct->GetFields();
		CHECK(fields.Size() == 3);

		CHECK(GetTypeID<u32>() == uintField->GetFieldInfo().typeInfo.typeId);
		CHECK(GetTypeID<String>() == stringField->GetFieldInfo().typeInfo.typeId);

		VoidPtr instance = testStruct->NewInstance();
		REQUIRE(instance != nullptr);

		static_cast<ReflectionTestStruct*>(instance)->iint = 100;

		CHECK(iintField->GetFieldPointer(instance) != nullptr);
		CHECK(iintField->GetFieldAs<i32>(instance) == 100);

		uintField->SetValueAs(instance, 10u);

		u32 vlCopy{};
		uintField->CopyValueTo(instance, &vlCopy);
		CHECK(vlCopy == 10);
		testStruct->Destroy(instance);

		Engine::Destroy();
	}

	TEST_CASE("Core::ReflectionFunctions")
	{
		Engine::Init();
		{
			auto reflectionFunctions = Registry::Type<ReflectionFunctions>();
			reflectionFunctions.Function<&ReflectionFunctions::VoidFunc>("VoidFunc").Attribute<TestAttribute>(101).Attribute<OtherTestAttribute>("Test");
			reflectionFunctions.Function<&ReflectionFunctions::ParamsRetFunc>("ParamsRetFunc").Attribute<TestAttribute>(202);
			reflectionFunctions.Function<&ReflectionFunctions::StaticFunc>("StaticFunc");
			reflectionFunctions.Function<&ReflectionFunctions::StaticFuncNoParam>("StaticFuncNoParam");
		}

		TypeHandler* handler = Registry::FindType<ReflectionFunctions>();
		REQUIRE(handler != nullptr);
		FunctionHandler* voidFunc = handler->FindFunction("VoidFunc");
		REQUIRE(voidFunc != nullptr);

		CHECK(voidFunc->GetName() == "VoidFunc");
		CHECK(voidFunc->GetParams().Empty());
		CHECK(voidFunc->GetReturn().typeInfo.typeId == GetTypeID<void>());
		CHECK(voidFunc->GetAttribute<TestAttribute>()->value == 101);
		CHECK(voidFunc->GetAttribute<OtherTestAttribute>()->value == "Test");

		ReflectionFunctions reflectionFunctions{};
		voidFunc->Invoke(&reflectionFunctions, nullptr, nullptr);
		CHECK(reflectionFunctions.calls == 1);

		typedef void(*FnVoidFunc)(const FunctionHandler* handler, VoidPtr instance);
		FnVoidFunc func = reinterpret_cast<FnVoidFunc>(voidFunc->GetFunctionPointer());
		func(voidFunc, &reflectionFunctions);
		CHECK(reflectionFunctions.calls == 2);

		FunctionHandler* paramRetFunc = handler->FindFunction("ParamsRetFunc");
		REQUIRE(paramRetFunc != nullptr);
		CHECK(paramRetFunc->GetReturn().typeInfo.typeId == GetTypeID<i32>());
		CHECK(!paramRetFunc->GetParams().Empty());
		CHECK(paramRetFunc->GetAttribute<TestAttribute>()->value == 202);
		CHECK(paramRetFunc->GetAttribute<OtherTestAttribute>() == nullptr);

		{
			i32 ret = 0;
			i32 a = 20;
			i32 b = 10;
			VoidPtr params[] = {&a, &b};
			paramRetFunc->Invoke(&reflectionFunctions, &ret, params);
			CHECK(reflectionFunctions.calls == 3);
			CHECK(ret == 30);
		}

		FunctionHandler* staticFunc = handler->FindFunction("StaticFunc");
		REQUIRE(staticFunc != nullptr);
		{
			i32 ret = 0;
			i32 a = 20;
			i32 b = 10;
			VoidPtr params[] = {&a, &b};
			staticFunc->Invoke(nullptr, &ret, params);
			CHECK(reflectionFunctions.calls == 3);
			CHECK(ret == 200);
			CHECK(ReflectionFunctions::staticVoidFuncCalls == 1);
		}

		FunctionHandler* staticFuncNoParam = handler->FindFunction("StaticFuncNoParam");
		REQUIRE(staticFuncNoParam != nullptr);
		staticFuncNoParam->Invoke(nullptr, nullptr, nullptr);
		CHECK(ReflectionFunctions::staticVoidFuncCalls == 2);

		{
			Span<FunctionHandler*> functions = Registry::FindFunctionsByAttribute<TestAttribute>();
			CHECK(functions.Size() == 2);
		}

		{
			Span<FunctionHandler*> functions = Registry::FindFunctionsByAttribute<OtherTestAttribute>();
			CHECK(functions.Size() == 1);
		}

		Engine::Destroy();
	}

	namespace
	{
		i32 GlobalSum(i32 a, i32 b)
		{
			return a + b;
		}
	}


	TEST_CASE("Core::ReflectionGlobalFunctions")
	{
		Engine::Init();
		{
			Registry::Function<&GlobalSum>("GlobalSum").Attribute<TestAttribute>();
		}

		{
			FunctionHandler* function = Registry::FindFunctionByName("GlobalSum");
			CHECK(function);

			i32 ret{}, a{10}, b{20};
			VoidPtr params[2]{&a, &b};
			function->Invoke(nullptr, &ret, params);
			CHECK(ret == 30);
		}

		{
			Span<FunctionHandler*> functions = Registry::FindFunctionsByAttribute<TestAttribute>();
			CHECK(functions.Size() == 1);
			CHECK(functions[0]->GetName() == "GlobalSum");
		}

		Engine::Destroy();
	}

	TEST_CASE("Core::ReflectionRuntimeTypes")
	{
		//TODO
//		constexpr TypeID RuntimeTestTypeId = HashValue("Tests::RuntimeTestType");
//		auto runtimeType = Registry::Type("Tests::RuntimeTestType", RuntimeTestTypeId);
//		runtimeType.Field("Value", GetTypeID<i32>());

	}
}