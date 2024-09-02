#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/World/World.hpp"
#include "Fyrion/World/Query.hpp"

using namespace Fyrion;

namespace
{
    struct TestChange
    {
        i32 number;

        bool operator==(const TestChange& testChange) const
        {
            return this->number == testChange.number;
        }
    };

    TEST_CASE("World::ChangeDetectionTest::Insert")
    {
        Engine::Init();
        {
            Registry::Type<TestChange>();
            World world;

            world.Spawn(TestChange{
                .number = 10
            });

            auto q1 = world.Query<Changed<TestChange>>();
            auto q2 = world.Query<Changed<TestChange>>();
            auto q3 = world.Query<Changed<TestChange>>();

            u32 count = 0;

            //fist time detecting change, this query should execute.
            q1.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 1);

            //sabe query again, in the same frame, it should not execute.
            q1.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 1);

            //same query, different instance, it should execute.
            q2.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 2);

            //updated executed.
            world.Update();

            //q3 never executed on the first frame. it should execute now.
            q3.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 3);

            //and should not execute again
            q3.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 3);

            //execute updated
            world.Update();

            //nothing should execute now.
            q1.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 3);

            q2.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 3);

            q3.ForEach([&](const TestChange& change)
            {
                count++;
            });

            CHECK(count == 3);

        }
        Engine::Destroy();
    }

    TEST_CASE("World::ChangeDetectionTest::Update")
    {
        Engine::Init();
        {
            Registry::Type<TestChange>();
            World world;

            world.Spawn(TestChange{
                .number = 10
            });

            auto q1 = world.Query<Changed<TestChange>>();
            auto q2 = world.Query<ReadWrite<TestChange>>();

            u32 count = 0;
            //first iteration with Change<> it should detect
            q1.ForEach([&](const TestChange& testChange)
            {
                CHECK(testChange.number == 10);
                count++;
            });

            CHECK(count == 1);

            world.Update();

            //second iteration with Change<> it should not detect
            q1.ForEach([&](const TestChange& testChange)
            {
                CHECK(testChange.number == 10);
                count++;
            });

            CHECK(count == 1);

            //first change.
            q2.ForEach([&](ReadWrite<TestChange> testChange)
            {
                testChange.value.number = 20;
            });

            //q1 should detect the change now.
            q1.ForEach([&](const TestChange& testChange)
            {
                CHECK(testChange.number == 20);
                count++;
            });

            CHECK(count == 2);

            //q1 should not detect the change now.
            q1.ForEach([&](const TestChange& testChange)
            {
                count++;
            });

            CHECK(count == 2);

        }
        Engine::Destroy();
    }
}
