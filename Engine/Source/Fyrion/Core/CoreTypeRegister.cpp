
#include "Math.hpp"
#include "Registry.hpp"
#include "UUID.hpp"

namespace Fyrion
{
    struct UUID;

    void RegisterBaseTypes()
    {
        Registry::Type<bool>("bool");
        Registry::Type<u8>("u8");
        Registry::Type<u16>("u16");
        Registry::Type<u32>("u32");
        Registry::Type<u64>("u64");
        Registry::Type<ul32>("ul32");
        Registry::Type<i8>("i8");
        Registry::Type<i16>("i16");
        Registry::Type<i32>("i32");
        Registry::Type<i64>("i64");
        Registry::Type<f32>("f32");
        Registry::Type<f64>("f64");
        Registry::Type<Array<u8>>("Fyrion::ByteArray");
        Registry::Type<String>("Fyrion::String");
        Registry::Type<StringView>("Fyrion::StringView");

        auto uuid = Registry::Type<UUID>();
        uuid.Field<&UUID::firstValue>("firstValue");
        uuid.Field<&UUID::secondValue>("secondValue");


        auto allocator = Registry::Type<Allocator>();
        allocator.Function<&Allocator::MemAlloc>("MemAlloc");
        allocator.Function<&Allocator::MemFree>("MemFree");
    }

    void RegisterMathTypes()
    {
        auto extent = Registry::Type<Extent>();
        extent.Field<&Extent::width>("width");
        extent.Field<&Extent::height>("height");

        auto vec2 = Registry::Type<Vec2>();
        vec2.Field<&Vec2::x>("x");
        vec2.Field<&Vec2::y>("y");

        auto vec3 = Registry::Type<Vec3>();
        vec3.Field<&Vec3::x>("x");
        vec3.Field<&Vec3::y>("y");
        vec3.Field<&Vec3::z>("z");

        auto vec4 = Registry::Type<Vec4>();
        vec4.Field<&Vec4::x>("x");
        vec4.Field<&Vec4::y>("y");
        vec4.Field<&Vec4::z>("z");
        vec4.Field<&Vec4::w>("w");

        auto quat = Registry::Type<Quat>();
        quat.Field<&Quat::x>("x");
        quat.Field<&Quat::y>("y");
        quat.Field<&Quat::z>("z");
        quat.Field<&Quat::w>("w");
    }

    void RegisterCoreTypes()
    {
        RegisterBaseTypes();
        RegisterMathTypes();
    }
}
