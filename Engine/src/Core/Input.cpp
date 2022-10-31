#include "Input.hpp"

#include "Math/Math.hpp"
#include "Core/Logger.hpp"

struct NH_API ButtonState
{
	bool pressed, changed, consumed;
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
	ButtonState& state = buttonStates[code];
	return state.pressed && !state.consumed;
}

bool Input::OnButtonDown(ButtonCode code)
{
	ButtonState& state = buttonStates[code];
	return state.pressed && state.changed && !state.consumed;
}

bool Input::OnButtonUp(ButtonCode code)
{
	ButtonState& state = buttonStates[code];
	return !state.pressed && state.changed && !state.consumed;
}

bool Input::OnButtonChange(ButtonCode code)
{
	ButtonState& state = buttonStates[code];
	return state.changed && !state.consumed;
}

const Vector2Int& Input::MousePos()
{
	return mousePos;
}

I16 Input::MouseWheelDelta()
{
	return mouseWheelDelta;
}

void Input::ConsumeInput(ButtonCode code)
{
	buttonStates[code].consumed = true;
}

void Input::ResetInput()
{
	anyButtonDown = false;

	for (ButtonState& state : buttonStates)
	{
		state.changed = false;
		state.consumed = false;
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