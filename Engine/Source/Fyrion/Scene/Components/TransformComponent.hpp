#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Scene/Component.hpp"

namespace Fyrion
{
    class FY_API TransformComponent : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        FY_FINLINE void SetPosition(const Vec3& p_position)
        {
            position = p_position;
            OnChange();
        }

        FY_FINLINE void SetRotation(const Quat& p_rotation)
        {
            rotation = p_rotation;
            OnChange();
        }

        FY_FINLINE void SetScale(const Vec3& p_scale)
        {
            scale = p_scale;
            OnChange();
        }

        FY_FINLINE void SetTransform(const Vec3& p_position, const Quat& p_rotation, const Vec3& p_scale)
        {
            position = p_position;
            rotation = p_rotation;
            scale = p_scale;
            OnChange();
        }

        FY_FINLINE void SetTransform(const Transform& transform)
        {
            SetTransform(transform.position, transform.rotation, transform.scale);
        }

        FY_FINLINE const Vec3& GetPosition() const
        {
            return position;
        }

        FY_FINLINE const Quat& GetRotation() const
        {
            return rotation;
        }

        FY_FINLINE const Vec3& GetScale() const
        {
            return scale;
        }

        FY_FINLINE const Mat4& GetWorldTransform() const
        {
            return worldTransform;
        }

        FY_FINLINE Mat4 GetLocalTransform() const
        {
            return Math::Translate(Mat4{1.0}, position) * Math::ToMatrix4(rotation) * Math::Scale(Mat4{1.0}, scale);
        }

        FY_FINLINE Transform GetTransform() const
        {
            return {position, rotation, scale};
        }

        void OnNotify(i64 type, VoidPtr userData) override;
        void OnChange() override;

        static void RegisterType(NativeTypeHandler<TransformComponent>& type);

    private:
        Vec3 position{0, 0, 0};
        Quat rotation{0, 0, 0, 1};
        Vec3 scale{1, 1, 1};

        Mat4 worldTransform{1.0};
    };
}
