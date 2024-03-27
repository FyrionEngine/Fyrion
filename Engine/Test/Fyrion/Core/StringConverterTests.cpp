#include "doctest.h"
#include "Fyrion/Core/StringConverter.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"

using namespace Fyrion;

namespace
{
	template<typename T>
	String TestToString(const T& value)
	{
		char  buffer[50];
		usize size = StringConverter<T>::ToString(buffer, 0, value);
		return {buffer, size};
	}

	template<typename T>
	T TestFromString(const StringView& str)
	{
		T value{};
		StringConverter<T>::FromString(str.Data(), str.Size(), value);
		return value;
	}


	TEST_CASE("Core::StringConverterBasics")
	{
		CHECK(TestToString(static_cast<u32>(9999)) == "9999");
		CHECK(TestToString(static_cast<i32>(9999)) == "9999");
		CHECK(TestToString(static_cast<i64>(9223372036854775804ll)) == "9223372036854775804");
		CHECK(TestToString(static_cast<u64>(18446744073709551615ull)) == "18446744073709551615");


		CHECK(TestFromString<u32>("9999") == 9999);
		CHECK(TestFromString<i32>("9999") == 9999);
		CHECK(TestFromString<i64>("9223372036854775804") == 9223372036854775804ll);
		CHECK(TestFromString<u64>("18446744073709551615") == 18446744073709551615ull);


		CHECK(TestFromString<f32>("9999.9") == 9999.9f);
		CHECK(TestFromString<f64>("9999.5") == 9999.5f);

		CHECK(TestToString(static_cast<f32>(10.1f)) == "10.100000");
		CHECK(TestToString(static_cast<f64>(10.1)) == "10.100000");
	}
}