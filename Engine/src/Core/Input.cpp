#include "Input.hpp"

struct ButtonState
{
    bool pressed, changed;
};

struct InputState
{
    ButtonState buttonStates[BUTTON_COUNT];
    bool anyButtonDown;
    I8 mouseWheelDelta;
};

static InputState* inputState;

bool Input::Initialize(void* state)
{
    inputState = (InputState*)state;
    inputState->anyButtonDown = false;
    inputState->mouseWheelDelta = 0;

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

//TODO: Vector2Int Input::MousePos() {}

I8 Input::MouseWheelDelta()
{
    return inputState->mouseWheelDelta;
}