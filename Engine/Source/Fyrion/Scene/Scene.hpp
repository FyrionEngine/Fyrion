#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"

namespace Fyrion
{
    class FY_API Scene
    {

    public:
        static void RegisterType(NativeTypeHandler<Scene>& type);
    private:
        String testStr = "blah blah blah";
    };
}
