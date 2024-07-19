#pragma once
#include "Fyrion/Scene/Component.hpp"

namespace Fyrion
{
    class MeshAsset;

    class MeshRender : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        static void RegisterType(NativeTypeHandler<MeshRender>& type);
    private:
        MeshAsset* mesh = nullptr;
    };
}
