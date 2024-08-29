#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Scene/Components/TransformComponent.hpp"
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


    u32 countBasicSystem = 0;
    u32 updateCount = 1;
    u32 changedOnUpdateCount = 0;
    u32 changeOnPostUpdateCount = 0;

    struct TestBasicSystem : System
    {
        FY_BASE_TYPES(System);

        Query<TestComponentOne, TestComponentTwo> query;
        u64 initFrame = 0;

        void OnInit(SystemSetup& setup) override
        {
            query = world->Query<TestComponentOne, TestComponentTwo>();

            for (u32 i = 0; i < 5; ++i)
            {
                world->Spawn(TestComponentOne{
                                 .intValue = i
                             },
                             TestComponentTwo{
                                 .value = 33,
                             }
                );

                initFrame = world->GetTickCount();
            }
        }

        void OnUpdate() override
        {
            u32 count = 0;
            query.ForEach([&](Entity entity, const TestComponentOne& one, const TestComponentTwo& two)
            {
                CHECK(two.value == 33);
                CHECK(one.intValue == count);
                countBasicSystem++;
                count++;
            });

            CHECK(countBasicSystem == 5 * updateCount);

            //this shoud in the next frame.
            world->Query<Changed<TestComponentOne>, TestComponentTwo>().ForEach([&](TestComponentTwo& testComponentTwo)
            {
                CHECK((initFrame + 1) == updateCount);
                changedOnUpdateCount++;
            });
        }


        void OnDestroy() override
        {

        }
    };

    struct BasicPostProcessing : System
    {
        FY_BASE_TYPES(System);

        void OnInit(SystemSetup& systemSetup) override
        {
            systemSetup.stage = SystemExecutionStage::OnPostUpdate;
        }

        void OnUpdate() override
        {
            //this shoud only in the same frame.
            world->Query<Changed<TestComponentOne>, TestComponentTwo>().ForEach([&](TestComponentTwo& testComponentTwo)
            {
                CHECK(world->GetTickCount()-1 == updateCount);
                changeOnPostUpdateCount++;
            });
        }
    };

    TEST_CASE("World::TestSystems")
    {
        Engine::Init();
        {
            Registry::Type<TestBasicSystem>();
            Registry::Type<BasicPostProcessing>();
            Registry::Type<TestComponentOne>();
            Registry::Type<TestComponentTwo>();

            World world;
            world.AddSystem<TestBasicSystem>();
            world.AddSystem<BasicPostProcessing>();

            for (int i = 0; i < 5; ++i)
            {
                world.Update();
                updateCount++;
            }

            CHECK(countBasicSystem == 5 * (updateCount -1));
            CHECK(changedOnUpdateCount == 5);
            CHECK(changeOnPostUpdateCount == 5);
        }
        Engine::Destroy();
    }
}
