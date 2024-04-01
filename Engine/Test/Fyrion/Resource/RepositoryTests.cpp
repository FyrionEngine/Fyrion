#include <doctest.h>

#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Engine.hpp"

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


    TEST_CASE("Resource::Basics")
    {
        Engine::Init();

        ResourceTypeBuilder<TestResource>::Builder()
            .Value<TestResource::Field1, i32>("Field1")
            .Value<TestResource::Field2, String>("Field2")
            .SubObject<TestResource::Subobject>("Subobject")
            .SubObjectSet<TestResource::SubobjectSet>("SubobjectSet")
            .Stream<TestResource::Stream>("Stream")
            .Build();

        RID testResource = Repository::CreateResource<TestResource>();
        CHECK(testResource);

        ResourceObject& write = Repository::Write(testResource);
        write[TestResource::Field1] = 20;
        write[TestResource::Field2] = String{"blabha"};

        i32 value = write[TestResource::Field1];
        const String& str = write[TestResource::Field2];




        write.Commit();




        Engine::Destroy();
    }
}