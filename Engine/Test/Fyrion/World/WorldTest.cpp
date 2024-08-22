#include <doctest.h>

#define FY_PERFORMANCE_TEST

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Chronometer.hpp"
#include "Fyrion/World/World.hpp"
#include "Fyrion/World/Query.hpp"

#ifdef FY_PERFORMANCE_TEST
#define ENTT_ID_TYPE Fyrion::u64;
#include "entt.hpp"
#endif

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

    struct TestComponentFour
    {
        u64 zzz;
    };

    TEST_CASE("World::Basic")
    {
        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();
            Registry::Type<TestComponentThree>();

            World world;
            Entity entity = world.Spawn(TestComponentOne{.value = 10}, TestComponentTwo{.value = 20, .strTest = LONG_TEXT});

            CHECK(world.Alive(entity));
            CHECK(entity > 0);

            CHECK(world.Get<TestComponentOne>(entity)->value == 10);
            CHECK(world.Get<TestComponentTwo>(entity)->value == 20);
            CHECK(world.Get<TestComponentTwo>(entity)->strTest == LONG_TEXT);

            Query<TestComponentOne, TestComponentTwo> query = world.Query<TestComponentOne, TestComponentTwo>();

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


#ifdef FY_PERFORMANCE_TEST
    TEST_CASE("World::TestMultipleEntities")
    {
        constexpr usize num = 1000000;

        Engine::Init();
        {
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();
            Registry::Type<TestComponentThree>();
            Registry::Type<TestComponentFour>();


            entt::registry ecs;
            World world;

            {
                Array<Entity> entities;
                entities.Reserve(num);

                {
                    Chronometer c;
                    for (u32 i = 0; i < num; ++i)
                    {
                        entities.EmplaceBack(world.Spawn(TestComponentOne{.value = i}, TestComponentTwo{.value = 20}, TestComponentThree{.aaa = 4444}));
                    }
                    c.Print("Fyrion - create three initial components");
                }

                {
                    Chronometer c;
                    for(Entity entity : entities)
                    {
                        world.Add(entity,TestComponentFour{.zzz = 444});
                    }
                    c.Print("Fyrion - add new component");
                }
            }

            {
                Array<entt::entity> entities;
                entities.Reserve(num);
                {
                    Chronometer c;
                    for (u32 i = 0; i < num; ++i)
                    {
                        auto entity = ecs.create();
                        ecs.emplace<TestComponentOne>(entity);
                        ecs.emplace<TestComponentTwo>(entity);
                        ecs.emplace<TestComponentThree>(entity);
                        entities.EmplaceBack(entity);
                    }
                    c.Print("Entt - create three initial components");
                }

                {
                    Chronometer c;
                    for(auto entity : entities)
                    {
                        ecs.emplace<TestComponentFour>(entity);
                    }
                    c.Print("Entt - add new component");
                }

            }
        }
        Engine::Destroy();
    }
#endif

}
