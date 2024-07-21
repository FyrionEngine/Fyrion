#include "FreeViewCamera.hpp"

#include "Fyrion/IO/Input.hpp"

namespace Fyrion
{
    FreeViewCamera::FreeViewCamera()
    {
        UpdateViewMatrix();
    }

    void FreeViewCamera::Process(f64 deltaTime)
    {
        if (active)
        {
            if (firstMouse)
            {
                Vec2 mousePos = Input::GetMousePosition();

               // Input::SetCursorEnabled(false);

                lastX = mousePos.x;
                lastY = mousePos.y;
                firstMouse = false;
            }

            if (Input::IsMouseMoving())
            {
                Vec2 mousePos = Input::GetMousePosition();

                float xOffset = mousePos.x - lastX;
                float yOffset = lastY - mousePos.y;

                lastX = mousePos.x;
                lastY = mousePos.y;

                float sensitivity = 0.1f;
                xOffset *= sensitivity;
                yOffset *= sensitivity;

                xOffset *= deltaTime;
                yOffset *= deltaTime;

                yaw += xOffset;
                pitch -= yOffset;

                if (pitch > Math::Radians(89.0f))
                {
                    pitch = Math::Radians(89.0f);
                }

                if (pitch < -Math::Radians(89.0f))
                {
                    pitch = -Math::Radians(89.0f);
                }

                Quat pitchRotation = Math::AngleAxis(pitch, Vec3{1.0f, 0.0f, 0.0f});
                Quat yawRotation = Math::AngleAxis(yaw, Vec3{0.0f, 1.0f, 0.0f});
                rotation = Math::Normalize(pitchRotation * yawRotation);
            }

            Vec3 movement{};

            f32 localCameraSpeed = cameraSpeed;

            if (Input::IsKeyDown(Key::LeftShift))
            {
                localCameraSpeed *= 3.f;
            }

            if (Input::IsKeyDown(Key::A))
            {
                movement += right * -localCameraSpeed;
            }
            if (Input::IsKeyDown(Key::D))
            {
                movement += right * localCameraSpeed;
            }
            if (Input::IsKeyDown(Key::W))
            {
                movement += direction * -localCameraSpeed;
            }
            if (Input::IsKeyDown(Key::S))
            {
                movement += direction * localCameraSpeed;
            }
            if (Input::IsKeyDown(Key::E))
            {
                movement += up * localCameraSpeed;
            }
            if (Input::IsKeyDown(Key::Q))
            {
                movement += up * -localCameraSpeed;
            }

            position += movement * deltaTime;

            UpdateViewMatrix();
        }
        else if (!firstMouse)
        {
            lastX = 0;
            lastY = 0;
            firstMouse = true;
            //Input::SetCursorEnabled(true);
        }
    }


    void FreeViewCamera::UpdateViewMatrix()
    {
        view = Math::ToMatrix4(rotation) * Math::Translate(position * -1.f) * Math::Scale(scale);
        right = {view[0][0], view[1][0], view[2][0]};
        up = {view[0][1], view[1][1], view[2][1]};
        direction = {view[0][2], view[1][2], view[2][2]};
    }
}
