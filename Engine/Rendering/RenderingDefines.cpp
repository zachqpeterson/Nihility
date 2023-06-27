#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Settings.hpp"
#include "Platform\Input.hpp"
#include "Core\Time.hpp"

// CAMERA

void Camera::Reset()
{
	targetYaw = 0.0f;
	targetPitch = 0.0f;

	targetMovement = position;

	mouseDragging = false;
	ignoreDraggingFrames = 3;
	mouseSensitivity = 1.0f;
}

void Camera::SetOrthograpic(F32 nearPlane_, F32 farPlane_, F32 viewportWidth_, F32 viewportHeight_, F32 zoom_)
{
	perspective = false;

	nearPlane = nearPlane_;
	farPlane = farPlane_;
	viewportWidth = viewportWidth_;
	viewportHeight = viewportHeight_;
	zoom = zoom_;

	updateProjection = true;
}

void Camera::SetPerspective(F32 nearPlane_, F32 farPlane_, F32 fov_, F32 aspectRatio_)
{
	perspective = true;

	nearPlane = nearPlane_;
	farPlane = farPlane_;
	fov = fov_;
	aspectRatio = aspectRatio_;

	updateProjection = true;
}

void Camera::SetAspectRatio(F32 aspectRatio_)
{
	aspectRatio = aspectRatio_;

	updateProjection = true;
}

const Matrix4& Camera::ViewProjection() const
{
	return viewProjection;
}

Vector4 Camera::Eye() const
{
	return { position.x, position.y, position.z, 1.0f };
}

const Vector3& Camera::Position() const
{
	return position;
}

Quaternion3 Camera::Rotation() const
{
	return Quaternion3{ Euler() };
}

Vector3 Camera::Euler() const
{
	return { pitch, yaw, 0.0f };
}

bool Camera::Perspective() const { return perspective; }

void Camera::SetPosition(const Vector3& p)
{
	targetMovement = p;
	position = p;
}

void Camera::SetRotation(const Quaternion3& q)
{
	F32 v = q.x * q.y + q.z * q.w;

	if (Math::Abs(v - 0.5f) < FLOAT_EPSILON)
	{
		targetPitch = HALF_PI_F * RAD_TO_DEG_F;
		targetYaw = 2.0f * Math::Atan2(q.x, q.w) * RAD_TO_DEG_F;
	}
	else if (Math::Abs(v + 0.5f) < FLOAT_EPSILON)
	{
		targetPitch = -HALF_PI_F * RAD_TO_DEG_F;
		targetYaw = -2.0f * Math::Atan2(q.x, q.w) * RAD_TO_DEG_F;
	}
	else
	{
		targetPitch = Math::Asin(2.0f * v) * RAD_TO_DEG_F;
		targetYaw = Math::Atan2(2.0f * (q.w * q.y - q.x * q.z), 1.0f - 2.0f * (q.y * q.y + q.z * q.z)) * RAD_TO_DEG_F;
	}

	pitch = targetPitch;
	yaw = targetYaw;
}

void Camera::SetRotation(const Vector3& rotation)
{
	targetPitch = rotation.x;
	targetYaw = rotation.y;
	pitch = targetPitch;
	yaw = targetYaw;
}

void Camera::Update()
{
	const Quaternion3 pitchRotation{ Vector3::Right, pitch };
	const Quaternion3 yawRotation{ Vector3::Up, yaw };
	const Quaternion3 rotation = (pitchRotation * yawRotation).Normalize();

	const Matrix4 translation{ position };
	view = rotation.ToMatrix4() * translation;

	right = { view[0][0], view[1][0], view[2][0] };
	up = { view[0][1], view[1][1], view[2][1] };
	forward = { view[0][2], view[1][2], view[2][2] };

	if (updateProjection)
	{
		updateProjection = false;

		if (perspective) { projection.SetPerspective(fov, aspectRatio, nearPlane, farPlane); }
		else { projection.SetOrthographic(zoom * -viewportWidth / 2.0f, zoom * viewportWidth / 2.0f, zoom * -viewportHeight / 2.0f, zoom * viewportHeight / 2.0f, nearPlane, farPlane); }
	}

	viewProjection = projection * view;

	if (Input::ButtonDragging(BUTTON_CODE_RIGHT_MOUSE))
	{
		if (ignoreDraggingFrames == 0)
		{
			I32 x, y;
			Input::MouseDelta(x, y);

			targetYaw -= x * mouseSensitivity * (F32)Time::DeltaTime() * RAD_TO_DEG_F;
			targetPitch -= y * mouseSensitivity * (F32)Time::DeltaTime() * RAD_TO_DEG_F;
		}
		else
		{
			--ignoreDraggingFrames;
		}

		mouseDragging = true;
	}
	else
	{
		mouseDragging = false;
		ignoreDraggingFrames = 3;
	}

	Vector3 cameraMovement{ 0, 0, 0 };
	F32 cameraMovementDelta = movementDelta;

	if (Input::ButtonDown(BUTTON_CODE_SHIFT)) { cameraMovementDelta *= 10.0f; }
	if (Input::ButtonDown(BUTTON_CODE_ALT)) { cameraMovementDelta *= 100.0f; }
	if (Input::ButtonDown(BUTTON_CODE_CTRL)) { cameraMovementDelta *= 0.1f; }

	if (Input::ButtonDown(BUTTON_CODE_LEFT) || Input::ButtonDown(BUTTON_CODE_A)) { cameraMovement += right * cameraMovementDelta; }
	if (Input::ButtonDown(BUTTON_CODE_RIGHT) || Input::ButtonDown(BUTTON_CODE_D)) { cameraMovement += right * -cameraMovementDelta; }
	if (Input::ButtonDown(BUTTON_CODE_UP) || Input::ButtonDown(BUTTON_CODE_W)) { cameraMovement += forward * -cameraMovementDelta; }
	if (Input::ButtonDown(BUTTON_CODE_DOWN) || Input::ButtonDown(BUTTON_CODE_S)) { cameraMovement += forward * cameraMovementDelta; }
	if (Input::ButtonDown(BUTTON_CODE_E)) { cameraMovement += up * -cameraMovementDelta; }
	if (Input::ButtonDown(BUTTON_CODE_Q)) { cameraMovement += up * cameraMovementDelta; }

	targetMovement += cameraMovement;

	const F32 tweenSpeed = rotationSpeed * (F32)Time::DeltaTime();

	pitch += (targetPitch - pitch) * tweenSpeed;
	yaw += (targetYaw - yaw) * tweenSpeed;

	const F32 tweenPositionSpeed = movementSpeed * (F32)Time::DeltaTime();
	position = Math::Lerp(position, targetMovement, 1.0f - Math::Pow(0.1f, (F32)Time::DeltaTime())); //TODO: Abstract
}