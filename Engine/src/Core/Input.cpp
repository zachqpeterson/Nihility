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

