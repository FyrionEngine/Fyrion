#include <doctest.h>

#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/TypeInfo.hpp"
#include "Fyrion/Engine.hpp"

using namespace Fyrion;

namespace
{
    using MyCustomEvent = EventType<"Event::MyCustomEvent"_h, void(i32 a, i32 b)>;

    i32 sumRes = 0;

    void Sum(i32 a, i32 b)
    {
        sumRes = a + b;
    }

    TEST_CASE("Core::EventsGlobalFunc")
    {
        Engine::Init();

        Event::Bind<MyCustomEvent, &Sum>();
        CHECK(Event::EventCount<MyCustomEvent>() == 1);

        EventHandler<MyCustomEvent> eventHandler{};
        eventHandler.Invoke(10, 20);
        CHECK(sumRes == 30);

        Event::Unbind<MyCustomEvent, &Sum>();
        CHECK(Event::EventCount<MyCustomEvent>() == 0);

        Engine::Destroy();
    }


    struct TestEventClass
    {
        i32 sumRes = 0;
        i32 otherValue = 0;

        static i32 sumResStatic;

        void Sum(i32 a, i32 b)
        {
            sumRes = a + b;
        }

        void SumConst(i32 a, i32 b) const
        {
            sumResStatic = otherValue + a + b;
        }
    };

    i32 TestEventClass::sumResStatic = 0;

    TEST_CASE("Core::EventsClassFunc")
    {
        Engine::Init();

        TestEventClass eventClass{
            .otherValue = 10
        };

        Event::Bind<MyCustomEvent, &TestEventClass::Sum>(&eventClass);
        Event::Bind<MyCustomEvent, &TestEventClass::SumConst>(&eventClass);

        CHECK(Event::EventCount<MyCustomEvent>() == 2);

        EventHandler<MyCustomEvent> eventHandler{};
        eventHandler.Invoke(40, 50);
        CHECK(eventClass.sumRes == 90);
        CHECK(eventClass.sumResStatic == 100);


        Event::Unbind<MyCustomEvent, &TestEventClass::Sum>(&eventClass);
        Event::Unbind<MyCustomEvent, &TestEventClass::SumConst>(&eventClass);

        CHECK(Event::EventCount<MyCustomEvent>() == 0);

        Engine::Destroy();
    }
}