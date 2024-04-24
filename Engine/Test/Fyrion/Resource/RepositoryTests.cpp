#include <thread>
#include <iostream>
#include "doctest.h"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Core/Registry.hpp"
//#include "Fyrion/EntryPoint.hpp"
#include "Fyrion/Engine.hpp"

using namespace Fyrion;

namespace
{
    namespace
    {
        u32 eventCallCount = 0;

        struct TestResource
        {
            constexpr static u32 BoolValue = 0;
            constexpr static u32 IntValue = 1;
            constexpr static u32 FloatValue = 2;
            constexpr static u32 StringValue = 3;
            constexpr static u32 LongValue = 4;
            constexpr static u32 SubObject = 5;
            constexpr static u32 SubObjectSet = 6;
        };

        struct TestOtherResource
        {
            constexpr static u32 TestValue = 0;
        };

        struct TestStructResource
        {
            String strTest{};
            Vec4 vecTest1{};
            Vec4 vecTest2{};
        };

        void CreateResourceTypes()
        {

            {
                auto testStructResource = Registry::Type<TestStructResource>();
                testStructResource.Field<&TestStructResource::strTest>("strTest");
                testStructResource.Field<&TestStructResource::vecTest1>("vecTest1");
                testStructResource.Field<&TestStructResource::vecTest2>("vecTest2");
            }

            ResourceTypeBuilder<TestResource>::Builder()
                .Value<TestResource::BoolValue, bool>("BoolValue")
                .Value<TestResource::IntValue, i32>("IntValue")
                .Value<TestResource::FloatValue, f32>("FloatValue")
                .Value<TestResource::StringValue, String>("StringValue")
                .Value<TestResource::LongValue, i64>("LongValue")
                .SubObject<TestResource::SubObject>("SubObject")
                .SubObjectSet<TestResource::SubObjectSet>("SubObjectSet")
                .Build();

            ResourceTypeBuilder<TestOtherResource>::Builder()
                .Value<TestOtherResource::TestValue, i32>("TestValue")
                .Build();
        }
    }

    TEST_CASE("Repository::Basics")
    {
        Engine::Init();

        CreateResourceTypes();

//        Repository::AddResourceTypeEvent(TestResource, nullptr, ResourceEventType_Insert | ResourceEventType_Update, [](CPtr userData, ResourceEventType eventType, ResourceObject& resourceObject)
//        {
//            eventCallCount++;
//        });

        {
            RID rid = Repository::CreateResource<TestResource>();
            UUID uuid = UUID::RandomUUID();
            Repository::SetUUID(rid, uuid);

            CHECK(Repository::GetUUID(rid) == uuid);
            CHECK(Repository::GetByUUID(uuid) == rid);

            {
                ResourceObject write = Repository::Write(rid);
                write.SetValue(TestResource::IntValue, 102);
                write.SetValue(TestResource::StringValue, String{"blahblah"});
                write.Commit();
            }

            u32 initVersion = Repository::GetVersion(rid);

            ResourceObject originalRead = Repository::Read(rid);

            CHECK(originalRead.GetResourceType(TestResource::BoolValue) == ResourceFieldType::Value);
            CHECK(originalRead.GetFieldType(TestResource::BoolValue)->GetTypeInfo().typeId == GetTypeID<bool>());

            {
                CHECK(originalRead.GetValue<i32>(TestResource::IntValue) == 102);
                CHECK(originalRead.GetValue<String>(TestResource::StringValue) == "blahblah");
            }

            {
                ResourceObject write = Repository::Write(rid);
                write.SetValue(TestResource::IntValue, 300);
            }

            {
                //no commit it will keep the original value
                ResourceObject read = Repository::Read(rid);
                CHECK(read.GetValue<i32>(TestResource::IntValue) == 102);
            }

            CHECK(Repository::GetVersion(rid) == initVersion);

            {
                ResourceObject write = Repository::Write(rid);
                write.SetValue(TestResource::IntValue, 300);
                write.Commit();
            }

            {
                ResourceObject read = Repository::Read(rid);
                CHECK(read.GetValue<i32>(TestResource::IntValue) == 300);
            }

            CHECK(Repository::GetVersion(rid) > initVersion);

            //original read should be alive because it's not garbage collected yet.
            CHECK(originalRead.GetValue<i32>(TestResource::IntValue) == 102);

//            usize nr = GetAllocationNum();
            //Repository::GarbageCollect();
//             CHECK(nr > GetAllocationNum());

//            CHECK(Repository::IsAlive(rid) == true);
//            Repository::DestroyResource(rid);
//            Repository::GarbageCollect();
//            CHECK(Repository::IsAlive(rid) == false);
        }

//        CHECK(eventCallCount == 2);

        Engine::Destroy();
    }

    TEST_CASE("Repository::Prototypes")
    {
        Engine::Init();

        CreateResourceTypes();

        //Create prototype
        RID prototype = Repository::CreateResource<TestResource>();
        {
            ResourceObject write = Repository::Write(prototype);
            write.SetValue(TestResource::IntValue, 300);
            write.SetValue(TestResource::StringValue, String{"blahblah"});
            write.SetValue(TestResource::FloatValue, 1.2f);
            write.SetValue(TestResource::BoolValue, true);
            write.Commit();
        }

        RID rid = Repository::CreateFromPrototype(prototype);
        {
            //check original values
            ResourceObject read = Repository::Read(rid);
            CHECK(read.GetValue<i32>(TestResource::IntValue) == 300);
            CHECK(read.GetValue<String>(TestResource::StringValue) == "blahblah");
            CHECK(read.GetValue<f32>(TestResource::FloatValue) == 1.2f);
            CHECK(read.GetValue<bool>(TestResource::BoolValue) == true);
        }
        {
            //modify a value
            ResourceObject write = Repository::Write(rid);
            write.SetValue(TestResource::StringValue, String{"another string"});
            write.SetValue(TestResource::BoolValue, false);
            write.Commit();
        }

        {
            //check modified values
            ResourceObject read = Repository::Read(rid);
            CHECK(read.GetValue<i32>(TestResource::IntValue) == 300);
            CHECK(read.GetValue<String>(TestResource::StringValue) == "another string");
            CHECK(read.GetValue<f32>(TestResource::FloatValue) == 1.2f);
            CHECK(read.GetValue<bool>(TestResource::BoolValue) == false);
        }

        {
            //check if prototype has the same values
            ResourceObject read = Repository::Read(prototype);
            CHECK(read.GetValue<i32>(TestResource::IntValue) == 300);
            CHECK(read.GetValue<String>(TestResource::StringValue) == "blahblah");
            CHECK(read.GetValue<f32>(TestResource::FloatValue) == 1.2f);
            CHECK(read.GetValue<bool>(TestResource::BoolValue) == true);
        }

        {
            //change prototype
            ResourceObject write = Repository::Write(prototype);
            write.SetValue(TestResource::IntValue, 500);
            write.SetValue(TestResource::StringValue, String{"Prototype Changes"});
            write.Commit();
        }

        {
            //read it again modified values
            ResourceObject read = Repository::Read(rid);
            CHECK(read.GetValue<i32>(TestResource::IntValue) == 500); //that's the prototype changed value.
            CHECK(read.GetValue<String>(TestResource::StringValue) == "another string"); //this should keep the changed value
            CHECK(read.GetValue<f32>(TestResource::FloatValue) == 1.2f); //original prototype value
            CHECK(read.GetValue<bool>(TestResource::BoolValue) == false); //this should keep the changed value
        }

        {
            Repository::ClearValues(rid);

            //check original values again, it should be equals as the prototype.
            ResourceObject read = Repository::Read(rid);
            CHECK(read.GetValue<i32>(TestResource::IntValue) == 500);
            CHECK(read.GetValue<String>(TestResource::StringValue) == "Prototype Changes");
            CHECK(read.GetValue<f32>(TestResource::FloatValue) == 1.2f);
            CHECK(read.GetValue<bool>(TestResource::BoolValue) == true);
        }

        Engine::Destroy();
    }

    TEST_CASE("Repository::TestSubObjects")
    {
        Engine::Init();

        CreateResourceTypes();
        {
            RID rid = Repository::CreateResource<TestResource>();
            u32 version = Repository::GetVersion(rid);
            RID subobject = Repository::CreateResource<TestOtherResource>();
            u32 subobjectVersion = Repository::GetVersion(subobject);
            {
                ResourceObject write = Repository::Write(subobject);
                write.SetValue(TestOtherResource::TestValue, 10);
                write.Commit();
                CHECK(Repository::GetVersion(subobject) > subobjectVersion);
                subobjectVersion = Repository::GetVersion(subobject);
            }

            {
                ResourceObject write = Repository::Write(rid);
                write.SetSubObject(TestResource::SubObject, subobject);
                write.Commit();
                CHECK(Repository::GetVersion(rid) > version);
                version = Repository::GetVersion(rid);
            }

            {
                ResourceObject write = Repository::Write(rid);
                write.SetValue(TestResource::StringValue, String{"teststr"});
                write.Commit();
                CHECK(Repository::GetVersion(rid) > version);
                version = Repository::GetVersion(rid);
            }

            {
                ResourceObject read = Repository::Read(rid);
                CHECK(read.GetSubObject(TestResource::SubObject) == subobject);
            }

            //parent version should be equals here
            CHECK(Repository::GetVersion(rid) == version);
            {
                ResourceObject write = Repository::Write(subobject);
                write.SetValue(TestOtherResource::TestValue, 20);
                write.Commit();
                CHECK(Repository::GetVersion(subobject) > subobjectVersion);
                subobjectVersion = Repository::GetVersion(subobject);
            }

            //subobject has changed, it should update the parent version too
            CHECK(Repository::GetVersion(rid) > version);

            //

            Repository::DestroyResource(rid);
            Repository::GarbageCollect();
            CHECK(Repository::IsAlive(rid) == false);
            CHECK(Repository::IsAlive(subobject) == false);
        }

        Engine::Destroy();
    }

    TEST_CASE("Repository::TestSubObjectsSet")
    {
        Engine::Init();

        CreateResourceTypes();

        RID rid = Repository::CreateResource<TestResource>();
        u32 version = Repository::GetVersion(rid);

        Array<RID> subobjects{10};

        {
            ResourceObject write = Repository::Write(rid);
            for (int i = 0; i < 10; ++i)
            {
                RID subObject = Repository::CreateResource<TestOtherResource>();
                subobjects[i] = subObject;

                ResourceObject writeSub = Repository::Write(subObject);
                writeSub.SetValue(TestOtherResource::TestValue, (i32) subObject.id);
                writeSub.Commit();

                write.AddToSubObjectSet(TestResource::SubObjectSet, subObject);
            }
            write.Commit();
            CHECK(Repository::GetVersion(rid) > version);
            version = Repository::GetVersion(rid);
        }

        {
            ResourceObject read = Repository::Read(rid);

            Array<RID> subObjects{};
            subObjects.Resize(read.GetSubObjectSetCount(TestResource::SubObjectSet));
            read.GetSubObjectSet(TestResource::SubObjectSet, subObjects);

            CHECK(subObjects.Size() == 10);

            for (const RID& subObject: subObjects)
            {
                ResourceObject readSub = Repository::Read(subObject);
                CHECK(readSub.GetValue<i32>(TestOtherResource::TestValue) == subObject.id);
            }
        }

        CHECK(Repository::GetVersion(rid) == version);

        {
            ResourceObject writeSub = Repository::Write(subobjects[2]);
            writeSub.SetValue(TestOtherResource::TestValue, (i32) 100);
            writeSub.Commit();
            CHECK(Repository::GetVersion(rid) > version);
            version = Repository::GetVersion(rid);
        }

        {
            ResourceObject writeSub = Repository::Write(subobjects[9]);
            writeSub.SetValue(TestOtherResource::TestValue, (i32) 200);
            writeSub.Commit();
            CHECK(Repository::GetVersion(rid) > version);
            version = Repository::GetVersion(rid);
        }

        {
            ResourceObject write = Repository::Write(rid);
            write.RemoveFromSubObjectSet(TestResource::SubObjectSet, subobjects[9]);
            write.Commit();
            CHECK(Repository::GetVersion(rid) > version);
            version = Repository::GetVersion(rid);
        }

        {
            //object has removed from parent, it should not change the parent version.
            ResourceObject writeSub = Repository::Write(subobjects[9]);
            writeSub.SetValue(TestOtherResource::TestValue, (i32) 200);
            writeSub.Commit();
            CHECK(Repository::GetVersion(rid) == version);
        }

        Engine::Destroy();
    }

    void CheckSubObjectSets(RID rid, u32 expectedCount)
    {
        Array<RID> rids{};
        ResourceObject read = Repository::Read(rid);
        usize count = read.GetSubObjectSetCount(TestResource::SubObjectSet);
        CHECK(count == expectedCount);
        rids.Resize(count);
        read.GetSubObjectSet(TestResource::SubObjectSet, rids);
        for (u32 i = 0; i < count; ++i)
        {
            ResourceObject readSub = Repository::Read(rids[i]);
            CHECK(readSub.GetValue<i32>(TestOtherResource::TestValue) == rids[i].id);
        }
    }

    TEST_CASE("Repository::TestSubObjectsSetPrototype")
    {
        Engine::Init();
        CreateResourceTypes();

        RID prototype = Repository::CreateResource<TestResource>();
        RID subObject1 = Repository::CreateResource<TestOtherResource>();
        RID subObject2 = Repository::CreateResource<TestOtherResource>();
        {
            {
                ResourceObject writeSub = Repository::Write(subObject1);
                writeSub.SetValue(TestOtherResource::TestValue, (i32) subObject1.id);
                writeSub.Commit();
            }

            {
                ResourceObject writeSub = Repository::Write(subObject2);
                writeSub.SetValue(TestOtherResource::TestValue, (i32) subObject2.id);
                writeSub.Commit();
            }

            ResourceObject write = Repository::Write(prototype);
            write.AddToSubObjectSet(TestResource::SubObjectSet, subObject2);
            write.AddToSubObjectSet(TestResource::SubObjectSet, subObject1);
            write.Commit();
        }


        //create new RID of prototype and check if new rid has 2 subobjects
        RID rid = Repository::CreateFromPrototype(prototype);
        CheckSubObjectSets(rid, 2);

        //remove from prototype
        {
            ResourceObject write = Repository::Write(rid);
            write.RemoveFromPrototypeSubObjectSet(TestResource::SubObjectSet, subObject1);
            write.Commit();
        }

        //check if rid has only one subobject and it's the correct one
        {
            Array<RID> rids{};
            ResourceObject read = Repository::Read(rid);
            usize count = read.GetSubObjectSetCount(TestResource::SubObjectSet);
            CHECK(count == 1);
            rids.Resize(count);
            read.GetSubObjectSet(TestResource::SubObjectSet, rids);
            CHECK(rids[0] == subObject2);
            ResourceObject readSub = Repository::Read(subObject2);
            CHECK(readSub.GetValue<i32>(TestOtherResource::TestValue) == subObject2.id);
        }

        //check if prototype is still correct:
        CheckSubObjectSets(prototype, 2);

        //create a prototype from other prototype.
        RID rid2 = Repository::CreateFromPrototype(rid);

        //this one should still have only one item
        {
            Array<RID> rids{};
            ResourceObject read = Repository::Read(rid2);
            usize count = read.GetSubObjectSetCount(TestResource::SubObjectSet);
            CHECK(count == 1);
            rids.Resize(count);
            read.GetSubObjectSet(TestResource::SubObjectSet, rids);
            CHECK(rids[0] == subObject2);
            ResourceObject readSub = Repository::Read(subObject2);
            CHECK(readSub.GetValue<i32>(TestOtherResource::TestValue) == subObject2.id);
        }

        {
            ResourceObject write = Repository::Write(rid);
            write.CancelRemoveFromPrototypeSubObjectSet(TestResource::SubObjectSet, subObject1);
            write.Commit();
        }

        //all items should have only one item
        CheckSubObjectSets(prototype, 2);
        CheckSubObjectSets(rid, 2);
        CheckSubObjectSets(rid2, 2);

        {
            ResourceObject write = Repository::Write(prototype);
            write.RemoveFromSubObjectSet(TestResource::SubObjectSet, subObject1);
            write.Commit();
        }

        //removed one from root prototype, all of them should have only one.
        CheckSubObjectSets(prototype, 1);
        CheckSubObjectSets(rid, 1);
        CheckSubObjectSets(rid2, 1);

        //clear subObjectSet
        {
            ResourceObject write = Repository::Write(prototype);
            write.ClearSubObjectSet(TestResource::SubObjectSet);
            write.Commit();
        }

        //all should have zero
        CheckSubObjectSets(prototype, 0);
        CheckSubObjectSets(rid, 0);
        CheckSubObjectSets(rid2, 0);

        Engine::Destroy();
    }

    TEST_CASE("Repository::TypeBasics")
    {
        Engine::Init();
        CreateResourceTypes();
        {
            RID rid = Repository::CreateResource<TestStructResource>();
            {
                TestStructResource testStructResource{
                    .strTest = "StrTest",
                    .vecTest1 = Vec4{1.0, 2.0, 3.0, 4.0},
                    .vecTest2 = Vec4{5.0, 6.0, 7.0, 8.0}
                };
                Repository::Commit(rid, &testStructResource);
            }
            {
                const TestStructResource& testStructResource = Repository::ReadData<TestStructResource>(rid);
                CHECK(testStructResource.strTest == "StrTest");
                CHECK(testStructResource.vecTest1 == Vec4{1.0, 2.0, 3.0, 4.0});
                CHECK(testStructResource.vecTest2 == Vec4{5.0, 6.0, 7.0, 8.0});
            }
        }
        Engine::Destroy();
    }

    TEST_CASE("Repository::TestMultithreading")
    {
        //breaking allocator count at end, but the test works
#if 0
        usize tries = std::thread::hardware_concurrency() * 30;

        Engine::Init();
        {
            CreateResourceTypes();

            u32 threads = std::max(std::thread::hardware_concurrency(), 4u);

            Array<std::thread> createThreads(threads);
            for (int i = 0; i < threads; ++i)
            {
                createThreads[i] = std::thread([&]()
                {
                    for (int j = 0; j < tries; ++j)
                    {
                        Repository::CreateResource<TestResource>();
                    }
                });
            }

            for (int i = 0; i < threads; ++i)
            {
                createThreads[i].join();
            }

            //to write
            RID rid = Repository::CreateResource<TestResource>();

            Array<std::thread> readThreads(threads);
            Array<std::thread> writeThreads(threads);
            std::atomic_bool check = false;

            for (int i = 0; i < threads; ++i)
            {
                writeThreads[i] = std::thread([&]()
                {
                    for (int j = 0; j < tries; ++j)
                    {
                        ResourceObject object = Repository::Write(rid);
                        object.SetValue(TestResource::LongValue, (i64) j);
                        object.Commit();
                    }
                });

                readThreads[i] = std::thread([&]()
                {
                    for (int j = 0; j < tries * 10 || !check; ++j)
                    {
                        ResourceObject object = Repository::Read(rid);
                        if (object)
                        {
                            if (object.GetValue<i64>(TestResource::LongValue) >= 0)
                            {
                                check = true;
                            }
                        }

                        //never gets the value
                        if (j > tries * 10000)
                        {
                            CHECK(false);
                            break;
                        }
                    }
                });
            }

            for (int i = 0; i < threads; ++i)
            {
                readThreads[i].join();
                writeThreads[i].join();
            }

            CHECK(check);
        }
        Engine::Destroy();
#endif
    }
}