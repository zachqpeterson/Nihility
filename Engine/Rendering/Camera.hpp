#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Math/Math.hpp"

enum class CameraType
{
	Orthographic,
	Perspective
};

struct NH_API Camera
{
public:
	bool Create(CameraType type = CameraType::Perspective);
	void Destroy();

	const F32& Near() const;
	const F32& Far() const;
	const Matrix4& View() const;
	const Matrix4& Projection() const;
	const Matrix4& ViewProjection() const;
	Vector4 Eye() const;
	const Vector3& Position() const;
	const F32& Zoom() const;
	const F32& FOV() const;
	Quaternion3 Rotation() const;
	Vector3 Euler() const;
	const Vector3& Right() const;
	const Vector3& Up() const;
	const Vector3& Forward() const;
	CameraType Type() const;

	void SetCameraType(CameraType type);
	void SetPosition(const Vector3& position);
	void SetRotation(const Quaternion3& rotation);
	void SetRotation(const Vector3& rotation);
	void SetZoom(F32 zoom);
	void SetFOV(F32 fov);
	void SetNearPlane(F32 nearPlane);
	void SetFarPlane(F32 farPlane);
	void SetViewport(const Vector2& viewport);

private:
	void Update();

	CameraType type;

	Matrix4 view = Matrix4::Identity;
	Matrix4 projection = Matrix4::Identity;
	Matrix4 viewProjection = Matrix4::Identity;

	Vector3 worldUp = Vector3::Up;
	Vector3 position = Vector3::Zero;
	Vector3 right = Vector3::Right;
	Vector3 forward = Vector3::Forward;
	Vector3 up = Vector3::Up;

	F32 pitch = 0.0f;
	F32 yaw = 0.0f;
	F32 roll = 0.0f;
	    
	F32 nearPlane = 0.0f;
	F32 farPlane = 0.0f;
	    
	F32 fov = 0.0f;
	F32 aspectRatio = 0.0f;
	    
	F32 zoom = 1.0f;
	F32 viewportWidth = 0.0f;
	F32 viewportHeight = 0.0f;

	friend class Renderer;
};