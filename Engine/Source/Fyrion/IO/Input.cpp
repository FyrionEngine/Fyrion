#include "Input.hpp"

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/FixedArray.hpp"
#include "Fyrion/Platform/Platform.hpp"

namespace Fyrion
{
    namespace
    {
        FixedArray<bool, static_cast<u64>(Key::MAX)>         keyState;
        FixedArray<bool, static_cast<u64>(Key::MAX)>         prevKeyState;
        FixedArray<bool, static_cast<u64>(MouseButton::MAX)> mouseButtonState;
        FixedArray<bool, static_cast<u64>(MouseButton::MAX)> prevMouseButtonState;
        Vec2                                                 mousePosition;
        bool                                                 mouseMoved = false;
    }

    bool Input::IsKeyDown(Key key)
    {
        return keyState[static_cast<u64>(key)];
    }

    bool Input::IsKeyPressed(Key key)
    {
        return keyState[static_cast<size_t>(key)] && !prevKeyState[static_cast<size_t>(key)];
    }

    bool Input::IsKeyReleased(Key key)
    {
        return !keyState[static_cast<size_t>(key)];
    }

    bool Input::IsMouseDown(MouseButton mouseButton)
    {
        return mouseButtonState[static_cast<size_t>(mouseButton)];
    }

    bool Input::IsMouseClicked(MouseButton mouseButton)
    {
        return mouseButtonState[static_cast<size_t>(mouseButton)] && !prevMouseButtonState[static_cast<size_t>(mouseButton)];
    }

    bool Input::IsMouseReleased(MouseButton mouseButton)
    {
        return !mouseButtonState[static_cast<size_t>(mouseButton)];
    }

    Vec2 Input::GetMousePosition()
    {
        return mousePosition;
    }

    bool Input::IsMouseMoving()
    {
        return mouseMoved;
    }

    void Input::RegisterInputEvent(InputEvent inputEvent)
    {
        switch (inputEvent.source)
        {
            case InputSourceType::Keyboard:
                keyState[static_cast<usize>(inputEvent.key)] = inputEvent.trigger == InputTriggerType::Pressed;
                break;
            case InputSourceType::Gamepad:
                break;
            case InputSourceType::MouseClick:
                mouseButtonState[static_cast<usize>(inputEvent.mouseButton)] = inputEvent.trigger == InputTriggerType::Pressed;
                break;
            case InputSourceType::MouseMove:
                mousePosition = inputEvent.value;
                mouseMoved = true;
                break;
        }
    }

    void InputBeginFrame()
    {
        mouseMoved = false;

        for (int i = 0; i < static_cast<u64>(Key::MAX); ++i)
        {
            prevKeyState[i] = keyState[i];
        }

        for (int i = 0; i < static_cast<u64>(MouseButton::MAX); ++i)
        {
            prevMouseButtonState[i] = mouseButtonState[i];
        }
    }

    void Input::SetCursorEnabled(bool enabled)
    {
        Platform::SetCursor(Engine::GetActiveWindow(), enabled ? MouseCursor::Arrow : MouseCursor::None);
    }

    void InputInit()
    {
        keyState = {};
        prevKeyState = {};
        prevMouseButtonState = {};
        mousePosition = {};

        Event::Bind<OnBeginFrame, InputBeginFrame>();
    }
}
