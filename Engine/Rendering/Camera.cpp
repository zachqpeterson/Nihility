#include "Camera.hpp"

#include "Platform/Input.hpp"

bool Camera::Create(CameraType type)
{
	this->type = type;

	switch (type)
	{
	case CameraType::Orthographic: {
		nearPlane = -100.0f;
		farPlane = 100.0f;
		viewportWidth = 120.0f;
		viewportHeight = 67.5f;
		zoom = 0.0f;
	} break;
	case CameraType::Perspective: {
		nearPlane = 0.01f;
		farPlane = 1000.0f;
		fov = 90.0f;
		aspectRatio = 1.7777778f;
	} break;
	}

	return true;
}

void Camera::Destroy()
{

}

void Camera::Update()
{
	const F32 zoomVal = Math::Pow(1.1f, zoom);

	const Quaternion3 pitchRotation = { Vector3::Right, pitch };
	const Quaternion3 yawRotation = { Vector3::Up, yaw };
	const Quaternion3 rollRotation = { Vector3::Forward, roll };
	const Quaternion3 rotation = (yawRotation * pitchRotation * rollRotation).Normalize();

	const Matrix4 translation = { position };
	view = translation * rotation.ToMatrix4();

	right = (Vector3)view[0];
	up = (Vector3)view[1];
	forward = (Vector3)view[2];

	switch (type)
	{
	case CameraType::Orthographic: {
		projection.SetOrthographic(zoomVal * -viewportWidth / 2.0f, zoomVal * viewportWidth / 2.0f, zoomVal * -viewportHeight / 2.0f, zoomVal * viewportHeight / 2.0f, nearPlane, farPlane);
	} break;
	case CameraType::Perspective: {
		projection.SetPerspective(fov, aspectRatio, nearPlane, farPlane);
	} break;
	}

	viewProjection = projection * view.Inverse();
}

const F32& Camera::Near() const
{
	return nearPlane;
}

const F32& Camera::Far() const
{
	return farPlane;
}

const Matrix4& Camera::View() const
{
	return view;
}

const Matrix4& Camera::Projection() const
{
	return projection;
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

const F32& Camera::Zoom() const
{
	return zoom;
}

const F32& Camera::FOV() const
{
	return fov;
}

Quaternion3 Camera::Rotation() const
{
	return Quaternion3{ Euler() };
}

Vector3 Camera::Euler() const
{
	return { pitch, yaw, roll };
}

const Vector3& Camera::Right() const
{
	return right;
}

const Vector3& Camera::Up() const
{
	return up;
}

const Vector3& Camera::Forward() const
{
	return forward;
}

CameraType Camera::Type() const
{
	return type;
}

void Camera::SetCameraType(CameraType type)
{
	Create(type);
}

void Camera::SetPosition(const Vector3& position)
{
	this->position = position;
}

void Camera::SetRotation(const Quaternion3& rotation)
{
	Vector3 euler = rotation.Euler();
	pitch = euler.x;
	yaw = euler.y;
	roll = euler.z;
}

void Camera::SetRotation(const Vector3& rotation)
{
	pitch = rotation.x;
	yaw = rotation.y;
	roll = rotation.z;
}

void Camera::SetZoom(F32 zoom)
{
	this->zoom = zoom;
}

void Camera::SetFOV(F32 fov)
{
	this->fov = fov;
}

void Camera::SetNearPlane(F32 nearPlane)
{
	this->nearPlane = nearPlane;
}

void Camera::SetFarPlane(F32 farPlane)
{
	this->farPlane = farPlane;
}

void Camera::SetViewport(const Vector2& viewport)
{
	viewportWidth = viewport.x;
	viewportHeight = viewport.y;
}
