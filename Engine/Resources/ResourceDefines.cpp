#include "ResourceDefines.hpp"

#include "Rendering\RenderingDefines.hpp"
#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"

// SAMPLER CREATION

SamplerInfo& SamplerInfo::SetMinMagMip(FilterType min, FilterType mag, SamplerMipmapMode mip)
{
	minFilter = min;
	magFilter = mag;
	mipFilter = mip;

	return *this;
}

SamplerInfo& SamplerInfo::SetBoundsModeU(SamplerBoundsMode u)
{
	boundsModeU = u;

	return *this;
}

SamplerInfo& SamplerInfo::SetBoundsModeUV(SamplerBoundsMode u, SamplerBoundsMode v)
{
	boundsModeU = u;
	boundsModeV = v;

	return *this;
}

SamplerInfo& SamplerInfo::SetBoundsModeUVW(SamplerBoundsMode u, SamplerBoundsMode v, SamplerBoundsMode w)
{
	boundsModeU = u;
	boundsModeV = v;
	boundsModeW = w;

	return *this;
}

// TEXTURE CREATION

TextureInfo& TextureInfo::SetSize(U16 width, U16 height, U16 depth)
{
	this->width = width;
	this->height = height;
	this->depth = depth;

	return *this;
}

TextureInfo& TextureInfo::SetFormatType(FormatType format, ImageType type)
{
	this->format = format;
	this->type = type;

	return *this;
}

TextureInfo& TextureInfo::SetName(const String& name)
{
	this->name = name;

	return *this;
}

TextureInfo& TextureInfo::SetData(void* data)
{
	initialData = data;

	return *this;
}

// RENDER PASS CREATION

Renderpass::Renderpass(Renderpass&& other) noexcept : renderpass(other.renderpass), frameBuffer(other.frameBuffer),
renderTargetCount(other.renderTargetCount), depthStencilTarget(other.depthStencilTarget)
{
	Copy(renderTargets, other.renderTargets, renderTargetCount);

	other.renderpass = nullptr;
	other.frameBuffer = nullptr;
}

Renderpass& Renderpass::operator=(Renderpass&& other) noexcept
{
	renderpass = other.renderpass;
	frameBuffer = other.frameBuffer;
	renderTargetCount = other.renderTargetCount;
	depthStencilTarget = other.depthStencilTarget;

	Copy(renderTargets, other.renderTargets, renderTargetCount);

	other.renderpass = nullptr;
	other.frameBuffer = nullptr;

	return *this;
}

RenderpassInfo& RenderpassInfo::Reset()
{
	renderTargetCount = 0;
	subpassCount = 0;

	depthStencilTarget = nullptr;

	colorLoadOp = ATTACHMENT_LOAD_OP_CLEAR;
	depthLoadOp = ATTACHMENT_LOAD_OP_CLEAR;
	stencilLoadOp = ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentFinalLayout = IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

	name.Clear();

	return *this;
}

RenderpassInfo& RenderpassInfo::AddSubpass(const Subpass& subpass)
{
	subpasses[subpassCount++] = subpass;

	return *this;
}

RenderpassInfo& RenderpassInfo::AddRenderTarget(const ResourceRef<Texture>& texture)
{
	renderTargets[renderTargetCount++] = texture;

	return *this;
}

RenderpassInfo& RenderpassInfo::SetDepthStencilTarget(const ResourceRef<Texture>& texture)
{
	depthStencilTarget = texture;

	return *this;
}

// CAMERA
void Camera::SetOrthograpic(F32 nearPlane, F32 farPlane, F32 viewportWidth, F32 viewportHeight, F32 zoom)
{
	//TODO: Warning if near and far plane are too far apart
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	this->viewportWidth = viewportWidth;
	this->viewportHeight = viewportHeight;
	SetZoom(zoom);

	type = CAMERA_TYPE_ORTHOGRAPHIC;
	updateProjection = true;
}

void Camera::SetPerspective(F32 nearPlane, F32 farPlane, F32 fov, F32 aspectRatio)
{
	//TODO: Warning if near and far plane are too far apart
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	this->fov = fov;
	this->aspectRatio = aspectRatio;

	type = CAMERA_TYPE_PERSPECTIVE;
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

const F32& Camera::ZoomVal() const
{
	return zoomVal;
}

const F32& Camera::Zoom() const
{
	return zoom;
}

const F32& Camera::InvZoom() const
{
	return invZoom;
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

CameraType Camera::Type() const { return type; }

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

void Camera::SetZoom(F32 zoom)
{
	zoomVal = zoom;
	this->zoom = Math::Pow(1.1f, zoom);
	invZoom = Math::Pow(1.1f, -zoom);
	updateProjection = true;
}

void Camera::SetFOV(F32 fov)
{
	this->fov = fov;
	updateProjection = true;
}

bool Camera::Update()
{
	if (updateView)
	{
		const Quaternion3 pitchRotation{ Vector3Right, pitch };
		const Quaternion3 yawRotation{ Vector3Up, yaw };
		const Quaternion3 rollRotation{ Vector3Forward, roll };
		const Quaternion3 rotation = (yawRotation * pitchRotation * rollRotation).Normalize();

		const Matrix4 translation{ position };
		view = translation * rotation.ToMatrix4();

		right = (Vector3)view[0];
		up = (Vector3)view[1];
		forward = (Vector3)view[2];
	}

	if (updateProjection)
	{
		switch (type)
		{
		case CAMERA_TYPE_PERSPECTIVE: {
			projection.SetPerspective(fov, aspectRatio, nearPlane, farPlane);
		} break;
		case CAMERA_TYPE_ORTHOGRAPHIC: {
			projection.SetOrthographic(zoom * -viewportWidth / 2.0f, zoom * viewportWidth / 2.0f, zoom * -viewportHeight / 2.0f, zoom * viewportHeight / 2.0f, nearPlane, farPlane);
		} break;
		}
	}

	if (updateProjection || updateView)
	{
		viewProjection = projection * view.Inverse();
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

void FlyCamera::SetPosition(const Vector3& position)
{
	camera.SetPosition(position);
}

void FlyCamera::SetRotation(const Quaternion3& rotation)
{
	camera.SetRotation(rotation);
}

void FlyCamera::SetRotation(const Vector3& rotation)
{
	camera.SetRotation(rotation);
}

Camera& FlyCamera::GetCamera()
{
	return camera;
}

Vector3 Tween(const Vector3& a, const Vector3& b, F32 power = 0.1f) noexcept { return Math::Lerp(a, b, 1.0f - Math::Pow(power, (F32)Time::DeltaTime())); }

bool FlyCamera::Update()
{
	switch (camera.Type())
	{
	case CAMERA_TYPE_PERSPECTIVE: {
		if (Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE))
		{
			if (ignoreDraggingFrames == 0)
			{
				Vector2 delta = Input::MouseDelta();

				targetYaw += delta.x * mouseSensitivity * (F32)Time::DeltaTime() * RAD_TO_DEG_F;
				targetPitch += delta.y * mouseSensitivity * (F32)Time::DeltaTime() * RAD_TO_DEG_F;
			}
			else
			{
				--ignoreDraggingFrames;
			}
		}
		else
		{
			ignoreDraggingFrames = 3;
		}

		Vector3 cameraMovement = Vector3Zero;
		F32 cameraMovementDelta = movementDelta;

		if (Input::ButtonDown(BUTTON_CODE_SHIFT)) { cameraMovementDelta *= 10.0f; }
		if (Input::ButtonDown(BUTTON_CODE_ALT)) { cameraMovementDelta *= 100.0f; }
		if (Input::ButtonDown(BUTTON_CODE_CTRL)) { cameraMovementDelta *= 0.1f; }

		if (Input::ButtonDown(BUTTON_CODE_A)) { cameraMovement += camera.Right() * -cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_D)) { cameraMovement += camera.Right() * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_W)) { cameraMovement += camera.Forward() * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_S)) { cameraMovement += camera.Forward() * -cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_E)) { cameraMovement += camera.Up() * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_Q)) { cameraMovement += camera.Up() * -cameraMovementDelta; }

		targetMovement += cameraMovement;

		const F32 tweenSpeed = rotationSpeed * (F32)Time::DeltaTime();

		Vector3 rotation = camera.Euler();
		rotation.x += (targetPitch - rotation.x) * tweenSpeed;
		rotation.y += (targetYaw - rotation.y) * tweenSpeed;
		camera.SetRotation(rotation);

		camera.SetPosition(Math::Lerp(camera.Position(), targetMovement, 1.0f - Math::Pow(0.1f, (F32)Time::DeltaTime()))); //TODO: Abstract
	} break;
	case CAMERA_TYPE_ORTHOGRAPHIC: {
		Vector3 cameraMovement = Vector3Zero;

		if (Input::ButtonDragging(BUTTON_CODE_RIGHT_MOUSE))
		{
			//TODO: dragging off screen to the left or top moves the camera super far
			Vector2 delta = Input::MouseDelta();
			Vector4 area = Renderer::RenderArea();

			cameraMovement.x -= delta.x * 0.125f * (1920 / area.z) * camera.Zoom();
			cameraMovement.y += delta.y * 0.125f * (1080 / area.w) * camera.Zoom();

			SetPosition(camera.Position() + cameraMovement);

			targetMovement = camera.Position();
			cameraMovement = Vector3Zero;
		}

		F32 cameraMovementDelta = movementDelta;

		if (Input::ButtonDown(BUTTON_CODE_SHIFT)) { cameraMovementDelta *= 10.0f; }
		if (Input::ButtonDown(BUTTON_CODE_ALT)) { cameraMovementDelta *= 100.0f; }
		if (Input::ButtonDown(BUTTON_CODE_CTRL)) { cameraMovementDelta *= 0.1f; }

		if (Input::ButtonDown(BUTTON_CODE_LEFT) || Input::ButtonDown(BUTTON_CODE_A)) { cameraMovement += Vector3Left * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_RIGHT) || Input::ButtonDown(BUTTON_CODE_D)) { cameraMovement += Vector3Right * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_UP) || Input::ButtonDown(BUTTON_CODE_W)) { cameraMovement += Vector3Up * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_DOWN) || Input::ButtonDown(BUTTON_CODE_S)) { cameraMovement += Vector3Down * cameraMovementDelta; }

		if (Input::MouseWheelDelta())
		{
			zoom -= Input::MouseWheelDelta();
			camera.SetZoom(zoom);
		}

		targetMovement += cameraMovement;

		camera.SetPosition(Tween(camera.Position(), targetMovement));

		if (Input::ButtonDown(BUTTON_CODE_CTRL) && Input::ButtonDown(BUTTON_CODE_R)) { zoom = 0; camera.SetZoom(0.0f); } //TODO: Standardized and changable hotkeys
		if (Input::ButtonDown(BUTTON_CODE_CTRL) && Input::ButtonDown(BUTTON_CODE_T)) { camera.SetPosition(Vector3Zero); targetMovement = 0.0f; } //TODO: Standardized and changable hotkeys
	} break;
	}

	return camera.Update();
}