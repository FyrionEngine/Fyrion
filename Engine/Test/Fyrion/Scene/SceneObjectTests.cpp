#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/Component.hpp"
#include "Fyrion/Scene/SceneManager.hpp"

namespace Fyrion
{
    class ComponentTest : public Component
    {
    public:
        inline static i32 destructorCall = 0;
        inline static i32 onDestroyCall = 0;

        i32 testValue = 0;
        i32 startCount = 0;
        i32 updateCount = 0;

        void OnStart() override
        {
            startCount++;
        }

        void OnUpdate(f64 deltaTime) override
        {
            updateCount++;
            if (updateCount > 1)
            {
                object->Destroy();
            }
        }

        void OnDestroy() override
        {
            onDestroyCall++;
        }

        ~ComponentTest() override
        {
            destructorCall++;
        };
    };

    TEST_CASE("Scene::BasicObjects")
    {
        Engine::Init();
        {
            EventHandler<OnUpdate> updateHandler{};

            Registry::Type<ComponentTest, Component>();

            SceneObject* object = SceneManager::CreateScene("TestScene");
            SceneManager::SetCurrenScene(object);

            REQUIRE(object);
            CHECK(object->GetScene() == object);

            SceneObject* child = object->NewChild("Child");
            REQUIRE(child);
            CHECK(child != object);

            {
                ComponentTest& componentTest = child->AddComponent<ComponentTest>();
                componentTest.testValue = 10;
            }

            {
                ComponentTest* componentTest =  child->GetComponent<ComponentTest>();
                REQUIRE(componentTest);
                CHECK(componentTest->testValue == 10);
                CHECK(componentTest->updateCount == 0);
            }

            updateHandler.Invoke(1.0);

            {
                ComponentTest* componentTest =  child->GetComponent<ComponentTest>();
                REQUIRE(componentTest);
                //CHECK(componentTest->startCount == 1);
                CHECK(componentTest->updateCount == 1);
            }

            updateHandler.Invoke(1.0);

            CHECK(ComponentTest::onDestroyCall == 1);
            CHECK(ComponentTest::destructorCall == 1);
        }
        Engine::Destroy();
    }
}
