#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/Component.hpp"
#include "Fyrion/Scene/SceneManager.hpp"

namespace Fyrion
{

    class ComponentToRemove : public Component
    {
    public:
        inline static i32 removed = 0;
        void OnDestroy() override;
    };

    class AnotherComponent : public Component
    {
    public:
        inline static i32 onStartCall = 0;
        void OnStart() override;
    };

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
            else
            {
                object->AddComponent<AnotherComponent>();
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


    void AnotherComponent::OnStart()
    {
        ComponentTest* componentTest = object->GetComponent<ComponentTest>();
        REQUIRE(componentTest);
        CHECK(componentTest->testValue == 10);

        CHECK(ComponentToRemove::removed == 0);
        object->RemoveComponent<ComponentToRemove>();
        CHECK(ComponentToRemove::removed == 1);

        onStartCall++;
    }

    void ComponentToRemove::OnDestroy()
    {
        removed++;
    }


    TEST_CASE("Scene::BasicObjects")
    {
        Engine::Init();
        {
            EventHandler<OnUpdate> updateHandler{};

            Registry::Type<ComponentTest, Component>();
            Registry::Type<AnotherComponent, Component>();
            Registry::Type<ComponentToRemove, Component>();

            SceneObject* object = SceneManager::CreateScene("TestScene");
            SceneManager::SetCurrenScene(object);
            CHECK(object->GetParent() == nullptr);

            REQUIRE(object);
            CHECK(object->GetScene() == object);

            SceneObject* child = object->NewChild("Child");
            REQUIRE(child);
            CHECK(child != object);

            CHECK(child->GetParent() == object);

            CHECK(object->GetChildren().Size() == 1);

            {
                ComponentTest& componentTest = child->AddComponent<ComponentTest>();
                componentTest.testValue = 10;
            }

            {
                ComponentTest* componentTest = child->GetComponent<ComponentTest>();
                REQUIRE(componentTest);
                CHECK(componentTest->testValue == 10);
                CHECK(componentTest->updateCount == 0);
            }

            updateHandler.Invoke(1.0);

            child->AddComponent<ComponentToRemove>();
            child->AddComponent<ComponentToRemove>();

            CHECK(child->GetComponentCount() == 4);
            CHECK(child->GetComponentTypeCount<ComponentToRemove>() == 2);
            CHECK(child->GetComponentTypeCount<ComponentTest>() == 1);


            {
                ComponentTest* componentTest = child->GetComponent<ComponentTest>();
                REQUIRE(componentTest);
                CHECK(componentTest->startCount == 1);
                CHECK(componentTest->updateCount == 1);
            }

            SceneObject* anotherChild = child->Duplicate();
            CHECK(anotherChild);

            CHECK(object->GetChildren().Size() == 2);

            updateHandler.Invoke(1.0);

            CHECK(ComponentTest::onDestroyCall == 1);
            CHECK(ComponentTest::destructorCall == 1);
            CHECK(AnotherComponent::onStartCall == 1);
            CHECK(ComponentToRemove::removed == 2);

            CHECK(object->GetChildren().Size() == 1);
        }
        Engine::Destroy();
    }
}