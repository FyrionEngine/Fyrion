#include <doctest.h>

#include "Fyrion/Repository/Repository.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Sinks.hpp"

using namespace Fyrion;

namespace
{
    enum class TestResource
    {
        Field1 = 0,
        Field2 = 1,
        Subobject = 2,
        SubobjectSet = 3,
        Stream = 4
    };


    TEST_CASE("Repository::Basics")
    {
        StdOutSink sink{};
        Logger::RegisterSink(sink);

        Engine::Init();

        ResourceTypeBuilder<TestResource>::Builder()
            .Value<TestResource::Field1, i32>("Field1")
            .Value<TestResource::Field2, String>("Field2")
            .SubObject<TestResource::Subobject>("Subobject")
            .SubObjectSet<TestResource::SubobjectSet>("SubobjectSet")
            .Stream<TestResource::Stream>("Stream")
            .Build();

        Engine::Destroy();
        Logger::Reset();

    }
}