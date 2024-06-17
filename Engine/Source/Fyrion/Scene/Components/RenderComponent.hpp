#pragma once
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion
{
    class FY_API RenderComponent : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        void        OnStart() override;
        static void RegisterType(NativeTypeHandler<RenderComponent>& type);
    private:
    };
}
