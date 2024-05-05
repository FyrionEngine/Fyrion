#pragma once
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion
{
    class FY_API RenderComponent : public Component
    {
    public:
        static void RegisterType(NativeTypeHandler<RenderComponent>& type);
    private:
    };
}
