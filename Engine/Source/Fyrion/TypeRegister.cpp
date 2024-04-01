#include "TypeRegister.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Math.hpp"

namespace Fyrion
{
    void RegisterResourceTypes();

    void TypeRegister()
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
        Registry::Type<String>("Skore::String");
        Registry::Type<StringView>("Skore::StringView");

        Registry::Type<RID>("Skore::RID");
        Registry::Type<Array<RID>>("Skore::RIDArray");

        auto extent = Registry::Type<Extent>();
        extent.Field<&Extent::width>("width");
        extent.Field<&Extent::height>("height");

        RegisterResourceTypes();
    }
}