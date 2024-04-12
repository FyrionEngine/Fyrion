
#include <doctest.h>
#include "Fyrion/Core/TypeInfo.hpp"

using namespace Fyrion;


namespace
{

    TEST_CASE("Core::GetSimpleName")
    {
        {
            StringView str = GetSimpleName("Core::Test::TestName");
            CHECK(str == "TestName");
        }

        {
            StringView str = GetSimpleName("TestName");
            CHECK(str == "TestName");
        }
    }
}
