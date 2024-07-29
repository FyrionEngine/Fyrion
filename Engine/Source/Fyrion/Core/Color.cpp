#include "Color.hpp"

#include "Registry.hpp"


namespace Fyrion
{
    template<> void TColor<u8>::RegisterType(NativeTypeHandler<Color>& type)
    {
        type.Field<&Color::red>("red");
        type.Field<&Color::green>("green");
        type.Field<&Color::blue>("blue");
        type.Field<&Color::alpha>("alpha");
    }
}
