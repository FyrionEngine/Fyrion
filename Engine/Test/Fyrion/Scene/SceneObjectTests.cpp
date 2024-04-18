#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Scene/SceneManager.hpp"

namespace Fyrion
{
    TEST_CASE("Scene::BasicObjects")
    {
        Engine::Init();
        {
            SceneObject* object = SceneManager::FindOrCreateScene("TestScene");
            REQUIRE(object);
            CHECK(object->GetScene() == object);
        }
        Engine::Destroy();

    }
}
