#pragma once
#include "Component.hpp"

namespace Fyrion
{
    class TextureAsset;

    class RenderComponent : public Component
    {
    public:
        FY_BASE_TYPES(Component);


        static void RegisterType(NativeTypeHandler<RenderComponent>& type);
    private:
        TextureAsset* texture = nullptr;
    };
}
