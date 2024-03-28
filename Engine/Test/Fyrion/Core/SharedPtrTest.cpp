#include "Fyrion/Core/SharedPtr.hpp"
#include "doctest.h"

using namespace Fyrion;

namespace
{
    usize constructorCount = 0;
    usize destructorCount = 0;

    struct SharedPtrType
    {

        SharedPtrType(i32 a, i32 b) : a(a), b(b)
        {
            constructorCount++;
        }

        i32 a{};
        i32 b{};

        virtual ~SharedPtrType()
        {
            destructorCount++;
        }
    };

    TEST_CASE("Core::SharedPtrBasics")
    {
        {
            auto ptr = MakeShared<SharedPtrType>(20, 30);
            CHECK(ptr != nullptr);
            CHECK(ptr->a == 20);
            CHECK(ptr->b == 30);
            CHECK(ptr.RefCount() == 1);

            auto otherPtr = ptr;
            CHECK(otherPtr != nullptr);
            CHECK(otherPtr->a == 20);
            CHECK(otherPtr->b == 30);

            CHECK(ptr.RefCount() == 2);

            auto movPtr = Traits::Move(otherPtr);
            CHECK(movPtr != nullptr);
            CHECK(movPtr->a == 20);
            CHECK(movPtr->b == 30);

            CHECK(otherPtr == nullptr);

            CHECK(ptr.RefCount() == 2);
        }

        CHECK(constructorCount == 1);
        CHECK(destructorCount == 1);

    }

}
