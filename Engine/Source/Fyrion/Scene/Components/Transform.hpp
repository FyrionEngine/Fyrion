#pragma once
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion
{
    class Transform : public Component
    {
    public:
        static void RegisterType(NativeTypeHandler<Transform>& type);
    private:
        Vec3 m_position{0, 0, 0};
        Quat m_rotation{0, 0, 0, 1};
        Vec3 m_scale{1, 1, 1};
    };
}
