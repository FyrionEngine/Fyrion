
#include <doctest.h>

#include "Fyrion/Common.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Any.hpp"

using namespace Fyrion;

namespace
{
    struct Test
    {
        i32 a{};

        static inline i32 count = 0;

        Test(const Test& other) : a(other.a)
        {
            count++;
        }

        Test()
        {
            count++;
        }

        explicit Test(i32 a) : a(a)
        {
            count++;
        }

        ~Test()
        {
            count--;
        }
    };

    TEST_CASE("Core::AnyBasics")
    {
        Engine::Init();
        {
            Registry::Type<Test>().Constructor<i32>();

            {
                Any any = MakeAny<Test>(10);
                CHECK(any.GetAs<Test>().a == 10);

                Any copy = any;
                CHECK(copy.GetAs<Test>().a == 10);
                copy.GetAs<Test>().a = 20;

                CHECK(any.GetAs<Test>().a == 10);
            }

            {
                Any any = MakeAny(Registry::FindType<Test>());
                CHECK(any.GetAs<Test>().a == 0);
                any.GetAs<Test>().a = 30;

                Any moved = Traits::Move(any);

                CHECK(!any);

                CHECK(moved);
                CHECK(moved.GetAs<Test>().a == 30);
            }

            CHECK(Test::count == 0);
        }
        Engine::Destroy();
    }
}