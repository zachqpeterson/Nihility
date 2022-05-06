#include "Input.hpp"

#include "Math/Math.hpp"

struct ButtonState
{
    bool pressed, changed;
};

struct InputState
{
    ButtonState buttonStates[BUTTON_COUNT];
    bool anyButtonDown;
    I16 mouseWheelDelta;
    Vector2Int mousePos;
};

static InputState* inputState;

bool Input::Initialize(void* state)
{
    inputState = (InputState*)state;
    inputState->anyButtonDown = false;
    inputState->mouseWheelDelta = 0;
    inputState->mousePos = 0;

    return true;
}

void* Input::Shutdown()
{
    return inputState;
}

const U64 Input::GetMemoryRequirements()
{
    return sizeof(InputState);
}

bool Input::OnAnyButtonDown()
{
    return inputState->anyButtonDown;
}

bool Input::ButtonDown(ButtonCode code)
{
    return inputState->buttonStates[code].pressed;
}

bool Input::OnButtonDown(ButtonCode code)
{
    ButtonState& btn = inputState->buttonStates[code];
    return btn.pressed && btn.changed;
}

bool Input::OnButtonUp(ButtonCode code)
{
    ButtonState& btn = inputState->buttonStates[code];
    return !btn.pressed && btn.changed;
}

const Vector2Int& Input::MousePos()
{
    return inputState->mousePos;
}

I8 Input::MouseWheelDelta()
{
    return inputState->mouseWheelDelta;
}

void Input::ResetInput()
{
    for (ButtonState state : inputState->buttonStates)
    {
        state.changed = false;
    }
}

void Input::SetButtonState(U8 code, bool down)
{
    inputState->buttonStates[code].changed = true;
    inputState->buttonStates[code].pressed = down;
}

void Input::SetMouseWheel(I16 delta)
{
    inputState->mouseWheelDelta = delta;
}

void Input::SetMousePos(I32 x, I32 y)
{
    Vector2Int& v = inputState->mousePos;
    v.x = x;
    v.y = y;
}