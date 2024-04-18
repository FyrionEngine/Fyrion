#pragma once
#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class GameObject;

    class FY_API Component
    {
    public:
        GameObject* object = nullptr;

        virtual ~Component() = default;

        virtual void OnStart() = 0;
        virtual void OnDestroy() = 0;
        virtual void OnUpdate(f64 deltaTime) = 0;

        void SetUpdateEnabled(bool updateEnabled);
    private:
        u64 m_updatableIndex = U64_MAX;
    };
}
