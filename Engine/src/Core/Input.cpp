#include "Input.hpp"

#include "Math/Math.hpp"
#include "Core/Logger.hpp"

Input::ButtonState Input::buttonStates[BUTTON_COUNT];
F32 Input::axisStates[GAMEPAD_AXIS_COUNT];
I16 Input::mouseWheelDelta;
Vector2Int Input::mousePos;
bool Input::anyButtonDown;

void Input::Update()
{
	for (U32 i = 0; i < BUTTON_COUNT; ++i)
	{
		anyButtonDown = false;
		buttonStates[i].changed = false;
		buttonStates[i].consumed = false;
		buttonStates[i].doubleClicked = false;
		buttonStates[i].heldChanged = false;
	}
}

bool Input::OnAnyButtonDown()
{
	return anyButtonDown;
}

bool Input::ButtonUp(ButtonCode code)
{
	return !buttonStates[code].pressed && !buttonStates[code].consumed;
}

bool Input::ButtonDown(ButtonCode code)
{
	return buttonStates[code].pressed && !buttonStates[code].consumed;
}

bool Input::ButtonHeld(ButtonCode code)
{
	return buttonStates[code].held && !buttonStates[code].consumed;
}

bool Input::OnButtonUp(ButtonCode code)
{
	return !buttonStates[code].pressed && buttonStates[code].changed && !buttonStates[code].consumed;
}

bool Input::OnButtonDown(ButtonCode code)
{
	return buttonStates[code].pressed && buttonStates[code].changed && !buttonStates[code].consumed;
}

bool Input::OnButtonChange(ButtonCode code)
{
	return buttonStates[code].changed && !buttonStates[code].consumed;
}

bool Input::OnButtonDoubleClick(ButtonCode code)
{
	return buttonStates[code].doubleClicked && !buttonStates[code].consumed;
}

bool Input::OnButtonHold(ButtonCode code)
{
	return buttonStates[code].held && buttonStates[code].heldChanged && !buttonStates[code].consumed;
}

bool Input::OnButtonRelease(ButtonCode code)
{
	return !buttonStates[code].held && buttonStates[code].heldChanged && !buttonStates[code].consumed;
}

void Input::ConsumeInput(ButtonCode code)
{
	buttonStates[code].consumed = true;
}

const Vector2Int& Input::MousePos()
{
	return mousePos;
}

I16 Input::MouseWheelDelta()
{
	return mouseWheelDelta;
}

F32 Input::GetAxis(GamepadAxis axis)
{
	return axisStates[axis];
}