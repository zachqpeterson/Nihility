#include "Input.hpp"

#include "Math/Math.hpp"
#include "Core/Logger.hpp"

struct NH_API ButtonState
{
    bool pressed, changed;
};

struct ButtonState Input::buttonStates[BUTTON_COUNT];
bool Input::anyButtonDown;
I16 Input::mouseWheelDelta;
Vector2Int Input::mousePos;

bool Input::Initialize()
{
    Logger::Info("Initializing input...");
    anyButtonDown = false;
    mouseWheelDelta = 0;
    mousePos = Vector2Int::ZERO;

    return true;
}

void Input::Shutdown()
{
    
}

bool Input::OnAnyButtonDown()
{
    return anyButtonDown;
}

bool Input::ButtonDown(ButtonCode code)
{
    return buttonStates[code].pressed;
}

bool Input::OnButtonDown(ButtonCode code)
{
    ButtonState& state = buttonStates[code];
    return state.pressed && state.changed;
}

bool Input::OnButtonUp(ButtonCode code)
{
    ButtonState& state = buttonStates[code];
    return !state.pressed && state.changed;
}

bool Input::OnButtonChange(ButtonCode code)
{
    return buttonStates[code].changed;
}

const Vector2Int& Input::MousePos()
{
    return mousePos;
}

I16 Input::MouseWheelDelta()
{
    return mouseWheelDelta;
}

void Input::ResetInput()
{
    anyButtonDown = false;

    for (ButtonState& state : buttonStates)
    {
        state.changed = false;
    }
}

void Input::SetButtonState(U8 code, bool down)
{
    anyButtonDown |= down;
    ButtonState& state = buttonStates[code];
    state.changed = down != state.pressed;
    state.pressed = down;
}

void Input::SetMouseWheel(I16 delta)
{
    mouseWheelDelta = delta;
}

void Input::SetMousePos(I32 x, I32 y)
{
    Vector2Int& v = mousePos;
    v.x = x;
    v.y = y;
}