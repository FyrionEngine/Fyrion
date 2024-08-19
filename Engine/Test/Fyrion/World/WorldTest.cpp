#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/World/World.hpp"

using namespace Fyrion;

namespace
{
    struct TestComponentOne
    {
        u32 value;
    };

    struct TestComponentTwo
    {
        u32 value;
    };

    TEST_CASE("World::Basic")
    {
        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();


            World  world;
            Entity entity = world.Spawn(TestComponentOne{.value = 10}, TestComponentTwo{.value = 20});
            CHECK(entity > 0);

            CHECK(world.Get<TestComponentOne>(entity).value == 10);
            CHECK(world.Get<TestComponentTwo>(entity).value == 20);
        }
        Engine::Destroy();
    }
}
