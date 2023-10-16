#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Settings.hpp"
#include "Platform\Input.hpp"
#include "Core\Time.hpp"

Descriptor::Descriptor() {}

Descriptor::Descriptor(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = range;
}

Descriptor::Descriptor(VkImageView imageView, VkImageLayout imageLayout, VkSampler sampler)
{
	imageInfo.imageView = imageView;
	imageInfo.imageLayout = imageLayout;
	imageInfo.sampler = sampler;
}

Descriptor::Descriptor(Texture* texture)
{
	imageInfo.imageView = texture->imageView;
	imageInfo.imageLayout = texture->imageLayout;
	if (texture->sampler) { imageInfo.sampler = texture->sampler->sampler; }
}

// CAMERA
void Camera::SetOrthograpic(F32 nearPlane, F32 farPlane, F32 viewportWidth, F32 viewportHeight, F32 zoom)
{
	//TODO: Warning if near and far plane are too far apart
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	this->viewportWidth = viewportWidth;
	this->viewportHeight = viewportHeight;
	this->zoom = zoom;

	perspective = false;
	updateProjection = true;
}

void Camera::SetPerspective(F32 nearPlane, F32 farPlane, F32 fov, F32 aspectRatio)
{
	//TODO: Warning if near and far plane are too far apart
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	this->fov = fov;
	this->aspectRatio = aspectRatio;

	perspective = true;
	updateProjection = true;
}

void Camera::SetAspectRatio(F32 aspectRatio)
{
	this->aspectRatio = aspectRatio;

	updateProjection = true;
}

const F32& Camera::Near() const
{
	return nearPlane;
}

const F32& Camera::Far() const
{
	return farPlane;
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

bool Camera::Perspective() const { return perspective; }

void Camera::SetPosition(const Vector3& p)
{
	position = p;
	updateView = true;
}

void Camera::SetRotation(const Quaternion3& q)
{
	Vector3 euler = q.Euler();
	pitch = euler.x;
	yaw = euler.y;
	roll = euler.z;

	updateView = true;
}

void Camera::SetRotation(const Vector3& rotation)
{
	pitch = rotation.x;
	yaw = rotation.y;
	roll = rotation.z;
	updateView = true;
}

bool Camera::Update()
{
	if (updateView)
	{
		const Quaternion3 pitchRotation{ Vector3::Right, pitch };
		const Quaternion3 yawRotation{ Vector3::Up, yaw };
		const Quaternion3 rollRotation{ Vector3::Forward, roll };
		const Quaternion3 rotation = (pitchRotation * yawRotation * rollRotation).Normalize();

		const Matrix4 translation{ position };
		view = rotation.ToMatrix4() * translation;

		right = { view[0][0], view[1][0], view[2][0] };
		up = { view[0][1], view[1][1], view[2][1] };
		forward = { view[0][2], view[1][2], view[2][2] };
	}

	if (updateProjection)
	{
		if (perspective) { projection.SetPerspective(fov, aspectRatio, nearPlane, farPlane); }
		else { projection.SetOrthographic(zoom * -viewportWidth / 2.0f, zoom * viewportWidth / 2.0f, zoom * -viewportHeight / 2.0f, zoom * viewportHeight / 2.0f, nearPlane, farPlane); }
	}

	if (updateProjection || updateView)
	{
		viewProjection = projection * view;
		updateProjection = false;
		updateView = false;

		return true;
	}

	return false;
}

void FlyCamera::SetOrthograpic(F32 nearPlane, F32 farPlane, F32 viewportWidth, F32 viewportHeight, F32 zoom)
{
	camera.SetOrthograpic(nearPlane, farPlane, viewportWidth, viewportHeight, zoom);
}

void FlyCamera::SetPerspective(F32 nearPlane, F32 farPlane, F32 fov, F32 aspectRatio)
{
	camera.SetPerspective(nearPlane, farPlane, fov, aspectRatio);
}

const Matrix4& FlyCamera::ViewProjection() const
{
	return camera.ViewProjection();
}

Vector4 FlyCamera::Eye() const
{
	return camera.Eye();
}

bool FlyCamera::Update()
{
	if (camera.Perspective())
	{
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

		if (Input::ButtonDown(BUTTON_CODE_LEFT) || Input::ButtonDown(BUTTON_CODE_A)) { cameraMovement += camera.Right() * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_RIGHT) || Input::ButtonDown(BUTTON_CODE_D)) { cameraMovement += camera.Right() * -cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_UP) || Input::ButtonDown(BUTTON_CODE_W)) { cameraMovement += camera.Forward() * -cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_DOWN) || Input::ButtonDown(BUTTON_CODE_S)) { cameraMovement += camera.Forward() * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_E)) { cameraMovement += camera.Up() * -cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_Q)) { cameraMovement += camera.Up() * cameraMovementDelta; }

		targetMovement += cameraMovement;

		const F32 tweenSpeed = rotationSpeed * (F32)Time::DeltaTime();

		Vector3 rotation = camera.Euler();
		rotation.x += (targetPitch - rotation.x) * tweenSpeed;
		rotation.y += (targetYaw - rotation.y) * tweenSpeed;
		camera.SetRotation(rotation);

		camera.SetPosition(Math::Lerp(camera.Position(), targetMovement, 1.0f - Math::Pow(0.1f, (F32)Time::DeltaTime()))); //TODO: Abstract
	}
	else
	{

	}

	return camera.Update();
}

void RenderGraph::AddPipeline(Pipeline* pipeline)
{
	U32 bestIndex = 0;
	bool found = false;

	U32 i = 0;
	for (Pass& pass : passes)
	{
		if (pipeline->renderpass == pass.renderpass)
		{
			found = true;
			i = 0;

			for (Pipeline* pl : pass.pipelines)
			{
				if (pipeline->shader->renderOrder < pl->shader->renderOrder)
				{
					pass.pipelines.Insert(i, pipeline);
				}
			}
		}
		else if (pipeline->renderpass->renderOrder < pass.renderpass->renderOrder) { bestIndex = i; }
	}

	if (!found)
	{
		passes.Insert(bestIndex, { pipeline->renderpass });
		passes[bestIndex].pipelines.Push(pipeline);
	}
}

void RenderGraph::Run(CommandBuffer* commandBuffer)
{
	for (Pass& pass : passes)
	{
		commandBuffer->BeginRenderpass(pass.renderpass);

		for (Pipeline* pipeline : pass.pipelines)
		{
			pipeline->Run(commandBuffer);
		}

		commandBuffer->EndRenderpass();
	}
}