#include <string>
#include "doctest.h"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"

using namespace Fyrion;

namespace
{

	TEST_CASE("Core::HashSetBasics")
	{
		HashSet<i32> set{};

		for (int i = 0; i < 1000; ++i)
		{
			set.Insert(i);
		}

		bool check = true;

		CHECK(set.Has(100));

		for (int i = 0; i < 1000; ++i)
		{
			HashSet<i32>::Iterator it = set.Find(i);
			if (it == set.end())
			{
				REQUIRE(false);
			}
		}

		CHECK(check);

		set.Clear();

		HashSet<i32>::Iterator it = set.Find(1);
		REQUIRE(it == set.end());
	}

	TEST_CASE("Core::HashSetForeach")
	{
		HashSet<i32> set{};
		set.Insert(20);
		set.Insert(40);

		i32 sum = 0;
		for(HashSet<i32>::Node& it : set)
		{
			sum += it.first;
		}
		CHECK(sum == 60);
	}

	TEST_CASE("Core::HashSetTestMove")
	{
		HashSet<i32> set{};
		set.Insert(20);
		set.Insert(40);

		HashSet<i32> other{Traits::Move(set)};
		CHECK(other.Has(40));
		CHECK(set.Empty());
	}

	TEST_CASE("Core::HashSetTestCopy")
	{
		HashSet<i32> set{};
		set.Insert(20);
		set.Insert(40);

		HashSet<i32> other = set;

		CHECK(set.Has(20));
		CHECK(set.Has(40));

		CHECK(other.Has(20));
		CHECK(other.Has(40));

		CHECK(other.Size() == 2);
	}

	TEST_CASE("Core::HashMapTestErase")
	{
		HashSet<i32> set{};
		set.Insert(20);
		set.Insert(40);

		set.Erase(20);

		CHECK(set.Find(20) == set.end());
		CHECK(set.Find(40) != set.end());

		set.Erase(40);

		CHECK(set.Find(2) == set.end());
	}

	TEST_CASE("Core::HashMapTestStr")
	{
		HashSet<String> set{};
		set.Insert("AAAA");
		set.Insert("CCCC");

		for (int i = 0; i < 10000; ++i)
		{
			std::string str = std::to_string(i);
			set.Insert(String{str.c_str()});
		}

		{
			auto it = set.Find("CCCC");
			REQUIRE(it != set.end());
		}

		{
			StringView strView = {"AAAA"};
			auto it = set.Find(strView);
			REQUIRE(it != set.end());
		}
	}

}