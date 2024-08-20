#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/World/World.hpp"

using namespace Fyrion;

namespace
{

#define LONG_TEXT "somebigtexttoheapallocatethestringadncheckifthedescturoisbeingcalled"

    struct TestComponentOne
    {
        u32 value;
    };

    struct TestComponentTwo
    {
        u32 value;
        String strTest;
    };

    TEST_CASE("World::Basic")
    {
        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();

            World world;
            Entity entity = world.Spawn(TestComponentOne{.value = 10}, TestComponentTwo{.value = 20, .strTest = LONG_TEXT});

            CHECK(world.Alive(entity));
            CHECK(entity > 0);

            CHECK(world.Get<TestComponentOne>(entity).value == 10);
            CHECK(world.Get<TestComponentTwo>(entity).value == 20);
            CHECK(world.Get<TestComponentTwo>(entity).strTest == LONG_TEXT);

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


    TEST_CASE("World::TestMultiple")
    {
        constexpr usize num = 20000;

        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();


            World world;
            for (int i = 0; i < num; ++i)
            {
                world.Spawn(TestComponentOne{.value = 10}, TestComponentTwo{.value = 20});
            }
            CHECK(world.Count() == num);

        }
        Engine::Destroy();
    }
}
