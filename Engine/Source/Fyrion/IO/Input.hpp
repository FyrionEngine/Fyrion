#pragma once

#include "InputTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Math.hpp"


namespace Fyrion::Input
{
    FY_API bool IsKeyDown(Key key);
    FY_API bool IsKeyPressed(Key key);
    FY_API bool IsKeyReleased(Key key);
    FY_API bool IsMouseDown(MouseButton mouseButton);
    FY_API bool IsMouseClicked(MouseButton mouseButton);
    FY_API bool IsMouseReleased(MouseButton mouseButton);
    FY_API Vec2 GetMousePosition();
    FY_API bool IsMouseMoving();
    FY_API void SetCursorEnabled(bool enabled);

    FY_API void RegisterInputEvent(InputEvent inputEvent);
}
