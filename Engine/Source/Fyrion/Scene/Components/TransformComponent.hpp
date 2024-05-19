#pragma once
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion
{
    class FY_API TransformComponent : public Component
    {
    public:

        FY_FINLINE void SetPosition(const Vec3& position)
        {
            m_position = position;
            NotifyTransformChange();
        }

        FY_FINLINE void SetRotation(const Quat& rotation)
        {
            m_rotation = rotation;
            NotifyTransformChange();
        }

        FY_FINLINE void SetScale(const Vec3& scale)
        {
            m_scale = scale;
            NotifyTransformChange();
        }

        FY_FINLINE void SetTransform(const Vec3& position, const Quat& rotation, const Vec3& scale)
        {
            m_position = position;
            m_rotation = rotation;
            m_scale = scale;
            NotifyTransformChange();
        }

        FY_FINLINE const Vec3& GetPosition() const { return m_position; }
        FY_FINLINE const Quat& GetRotation() const { return m_rotation; }
        FY_FINLINE const Vec3& GetScale() const { return m_scale; }
        FY_FINLINE const Mat4& GetWorldTransform() const { return m_worldTransform; }

        void OnNotify(i64 type) override;

        inline static i64 TransformChanged = 1001;

        static void RegisterType(NativeTypeHandler<TransformComponent>& type);
    private:
        Vec3 m_position{0, 0, 0};
        Quat m_rotation{0, 0, 0, 1};
        Vec3 m_scale{1, 1, 1};

        Mat4 m_worldTransform{1.0};

        void NotifyTransformChange() const;
    };
}
