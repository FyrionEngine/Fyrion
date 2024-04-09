
#include "Fyrion/Core/UniquePtr.hpp"
#include "doctest.h"
#include "Fyrion/Core/Array.hpp"

using namespace Fyrion;

namespace
{
    struct TestUniquePtr
    {
        i32 value{};
        inline static i32 destructorCount = 0;

        explicit TestUniquePtr(i32 value) : value(value)
        {
        }

        virtual ~TestUniquePtr()
        {
            destructorCount++;
        }
    };


    TEST_CASE("Core::UniquePtrBasics")
    {

        {
            UniquePtr<TestUniquePtr> vl = MakeUnique<TestUniquePtr>(10);
            CHECK(vl->value == 10);
        }

        CHECK(TestUniquePtr::destructorCount == 1);

        {
            Array<UniquePtr<TestUniquePtr>> ptrs = {};
            ptrs.Resize(1);
            CHECK(!ptrs[0]);
            ptrs.EmplaceBack(MakeUnique<TestUniquePtr>(100));
            CHECK(ptrs[1]);
            CHECK(ptrs[1]->value == 100);
        }

        CHECK(TestUniquePtr::destructorCount == 2);
    }
}