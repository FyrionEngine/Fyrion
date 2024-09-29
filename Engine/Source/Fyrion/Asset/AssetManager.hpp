#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    class FY_API AssetManager
    {
    public:


        template<typename T>
        static T* LoadByPath(StringView path)
        {
            return nullptr;
        }


    private:
    };
}
