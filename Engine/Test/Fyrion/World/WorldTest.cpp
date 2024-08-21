#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/World/World.hpp"

#define ENTT_ID_TYPE Fyrion::u64;
#include "entt.hpp"

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
        u32    value;
        String strTest;
    };

    struct TestComponentThree
    {
        u64 aaa;
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
        constexpr usize num = 1000;

        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();
            Registry::Type<TestComponentThree>();


            entt::registry ecs;

            World world;
            u64   sum = 0;
            for (u32 i = 0; i < num; ++i)
            {
                world.Spawn(TestComponentOne{.value = i}, TestComponentTwo{.value = 20}, TestComponentThree{.aaa = 0});
                sum += i;
            }

            CHECK(world.Count() == num);


            for (u32 i = 0; i < num; ++i)
            {
                auto entity = ecs.create();
                ecs.emplace<TestComponentOne>(entity);
                ecs.emplace<TestComponentTwo>(entity);
                ecs.emplace<TestComponentThree>(entity);
            }


            {
                Logger::GetLogger("test").Critical("Before Entt");
                u64 sum2 = 0;
                ecs.view<TestComponentOne, TestComponentTwo, TestComponentThree>().each([&](entt::entity entity, TestComponentOne& comp, TestComponentTwo& two, TestComponentThree& tree)
                {
                    sum2 += comp.value;
                });
                Logger::GetLogger("test").Critical("After Entt");
            }

            {
                Logger::GetLogger("test").Critical("Before Fyrion");
                u64 sum2 = 0;
                world.Query<TestComponentOne, TestComponentTwo, TestComponentThree>().Each([&](Entity entity, const TestComponentOne& comp, const TestComponentTwo& two, const TestComponentThree& tree)
                {
                    sum2 += comp.value;
                });
                Logger::GetLogger("test").Critical("After Fyrion");

                CHECK(sum == sum2);
            }
        }
        Engine::Destroy();
    }
}
