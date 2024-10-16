#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/IO/Asset.hpp"

namespace Fyrion
{
    class FY_API Scene : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<Scene>& type);

    private:
        String testStr = "blah blah blah";
    };
}
