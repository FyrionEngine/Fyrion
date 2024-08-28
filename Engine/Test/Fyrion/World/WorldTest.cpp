#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Chronometer.hpp"
#include "Fyrion/World/World.hpp"
#include "Fyrion/World/Query.hpp"

using namespace Fyrion;

namespace
{
#define LONG_TEXT "somebigtexttoheapallocatethestringadncheckifthedescturoisbeingcalled"

    struct TestComponentOne
    {
        u32 intValue;

        bool operator==(const TestComponentOne& one) const
        {
            return this->intValue == one.intValue;
        }
    };

    struct TestComponentTwo
    {
        u32    value;
        String strTest;
    };


    TEST_CASE("World::Basic")
    {
        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();

            World  world;
            Entity entity = world.Spawn(TestComponentOne{.intValue = 10}, TestComponentTwo{.value = 20, .strTest = LONG_TEXT});

            CHECK(world.Alive(entity));
            CHECK(entity > 0);

            CHECK(world.Get<TestComponentOne>(entity)->intValue == 10);
            CHECK(world.Get<TestComponentTwo>(entity)->value == 20);
            CHECK(world.Get<TestComponentTwo>(entity)->strTest == LONG_TEXT);

            u32 count = 0;

            world.Query<TestComponentOne, TestComponentTwo>().ForEach([&](const TestComponentOne& componentOne, const TestComponentTwo& componentTwo)
            {
                CHECK(componentOne.intValue == 10);
                CHECK(componentTwo.value == 20);
                CHECK(componentTwo.strTest == LONG_TEXT);
                count++;
            });

            CHECK(count == 1);

            world.Remove<TestComponentTwo>(entity);

            CHECK(!world.Has<TestComponentTwo>(entity));
            CHECK(world.Has<TestComponentOne>(entity));

            world.Destroy(entity);

            CHECK(!world.Has<TestComponentTwo>(entity));
            CHECK(!world.Has<TestComponentOne>(entity));

            CHECK(!world.Alive(entity));

            //it should recicle the entity id
            CHECK(world.Spawn() == entity);
        }
        Engine::Destroy();
    }


    struct TestSystem : System
    {
        FY_BASE_TYPES(System);

        void OnInit(SystemSetup& setup) override
        {
            for (u32 i = 0; i < 10; ++i)
            {
                world->Spawn(TestComponentOne{
                    .intValue = i
                });
            }
        }

        void OnUpdate() override
        {
            //WithAll<>

            // QueryBuilder builder;
            // builder.Changed<TestComponentOne>().WithAll<TestComponentOne, TestComponentTwo>().Build();
            //
            // world->ForEach([](RefMut<TestComponentOne>& one, const TestComponentTwo& testComponentTwo)
            // {
            //     one = TestComponentOne{
            //         .intValue = testComponentTwo.value
            //     };
            // });

            //Query<TestComponentOne, TestComponentTwo>()
        }

        void OnDestroy() override
        {

        }
    };

    TEST_CASE("World::TestSystems")
    {
        Engine::Init();
        {
            Registry::Type<TestSystem>();
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();

            World world;
            world.AddSystem<TestSystem>();
            world.Update();
        }
        Engine::Destroy();
    }
}
