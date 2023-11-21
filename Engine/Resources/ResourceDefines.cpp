#include "ResourceDefines.hpp"

#include "Rendering\RenderingDefines.hpp"
#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Core\Time.hpp"

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

// RENDER PASS

void Renderpass::Resize()
{
	if (lastResize < Renderer::absoluteFrame)
	{
		lastResize = Renderer::absoluteFrame;

		for (U32 i = 0; i < renderTargetCount; ++i)
		{
			Resources::RecreateTexture(renderTargets[i], Settings::WindowWidth(), Settings::WindowHeight(), 1);
		}

		if (depthStencilTarget)
		{
			Resources::RecreateTexture(depthStencilTarget, Settings::WindowWidth(), Settings::WindowHeight(), 1);
		}

		vkDestroyFramebuffer(Renderer::device, frameBuffer, Renderer::allocationCallbacks);
		vkDestroyRenderPass(Renderer::device, renderpass, Renderer::allocationCallbacks);

		Renderer::CreateRenderpass(this);
	}
}

// RENDER PASS CREATION

RenderpassInfo& RenderpassInfo::Reset()
{
	renderTargetCount = 0;

	depthStencilTarget = nullptr;

	colorLoadOp = ATTACHMENT_LOAD_OP_CLEAR;
	depthLoadOp = ATTACHMENT_LOAD_OP_CLEAR;
	stencilLoadOp = ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentFinalLayout = IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

	name.Clear();

	return *this;
}

RenderpassInfo& RenderpassInfo::AddRenderTarget(Texture* texture)
{
	renderTargets[renderTargetCount++] = texture;

	return *this;
}

RenderpassInfo& RenderpassInfo::SetDepthStencilTarget(Texture* texture)
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

const F32& Camera::Zoom()
{
	return zoom;
}

const F32& Camera::FOV()
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

void Camera::SetZoom(F32 zoom)
{
	this->zoom = zoom;
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

Camera* FlyCamera::GetCamera()
{
	return &camera;
}

bool FlyCamera::Update()
{
	if (camera.Perspective())
	{
		if (Input::ButtonDragging(BUTTON_CODE_RIGHT_MOUSE))
		{
			if (ignoreDraggingFrames == 0)
			{
				F32 x, y;
				Input::MouseDeltaPrecise(x, y);

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

		Vector3 cameraMovement = Vector3Zero;
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
		Vector3 cameraMovement = Vector3Zero;

		if (Input::ButtonDragging(BUTTON_CODE_RIGHT_MOUSE))
		{
			if (ignoreDraggingFrames == 0)
			{
				F32 x, y;
				Input::MouseDeltaPrecise(x, y);

				cameraMovement.x += x * camera.Zoom();
				cameraMovement.y -= y * camera.Zoom();

				SetPosition(camera.Position() + cameraMovement);

				targetMovement = camera.Position();
				cameraMovement = Vector3Zero;
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

		F32 cameraMovementDelta = movementDelta;

		if (Input::ButtonDown(BUTTON_CODE_SHIFT)) { cameraMovementDelta *= 10.0f; }
		if (Input::ButtonDown(BUTTON_CODE_ALT)) { cameraMovementDelta *= 100.0f; }
		if (Input::ButtonDown(BUTTON_CODE_CTRL)) { cameraMovementDelta *= 0.1f; }

		if (Input::ButtonDown(BUTTON_CODE_LEFT) || Input::ButtonDown(BUTTON_CODE_A)) { cameraMovement += Vector3Right * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_RIGHT) || Input::ButtonDown(BUTTON_CODE_D)) { cameraMovement += Vector3Left * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_UP) || Input::ButtonDown(BUTTON_CODE_W)) { cameraMovement += Vector3Down * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_DOWN) || Input::ButtonDown(BUTTON_CODE_S)) { cameraMovement += Vector3Up * cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_E)) { cameraMovement += Vector3Forward * -cameraMovementDelta; }
		if (Input::ButtonDown(BUTTON_CODE_Q)) { cameraMovement += Vector3Forward * cameraMovementDelta; }

		if (Input::MouseWheelDelta()) { camera.SetZoom(camera.Zoom() - Input::MouseWheelDelta() * 0.5f * cameraMovementDelta); }

		targetMovement += cameraMovement;

		camera.SetPosition(Math::Lerp(camera.Position(), targetMovement, 1.0f - Math::Pow(0.1f, (F32)Time::DeltaTime()))); //TODO: Abstract
	}

	return camera.Update();
}

void PipelineGraph::Destroy()
{
	for (Vector<PipelineInfo>& pipelineInfos : infos)
	{
		for (PipelineInfo& info : pipelineInfos) { info.Destroy(); }

		pipelineInfos.Destroy();
	}
	for (Pass& p : passes) { p.pipelines.Destroy(); }

	infos.Destroy();
	passes.Destroy();
}

void PipelineGraph::Create(const String& name)
{
	passes.Resize(infos.Size());
	bool first = true;

	if (!renderTarget)
	{
		TextureInfo textureInfo{};
		textureInfo.name = name + "_render_target";
		textureInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		textureInfo.width = Settings::WindowWidth();
		textureInfo.height = Settings::WindowHeight();
		textureInfo.depth = 1;
		textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
		textureInfo.type = VK_IMAGE_TYPE_2D;
		renderTarget = Resources::CreateTexture(textureInfo);
	}
	else
	{
		Resources::RecreateTexture(renderTarget, Settings::WindowWidth(), Settings::WindowHeight(), 1);
	}

	if (!depthTarget)
	{
		TextureInfo textureInfo{};
		textureInfo.name = name + "_depth_target";
		textureInfo.format = VK_FORMAT_D32_SFLOAT;
		textureInfo.width = Settings::WindowWidth();
		textureInfo.height = Settings::WindowHeight();
		textureInfo.depth = 1;
		textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
		textureInfo.type = VK_IMAGE_TYPE_2D;
		depthTarget = Resources::CreateTexture(textureInfo);
	}
	else
	{
		Resources::RecreateTexture(depthTarget, Settings::WindowWidth(), Settings::WindowHeight(), 1);
	}

	String passName = name + "pass";

	//TODO: Only create new renderpasses/pipelines in this is a recreation
	U32 subpass = 0;
	U32 i = 0;
	for (Vector<PipelineInfo>& info : infos)
	{
		passes[i].pipelines.Resize(info.Size());

		RenderpassInfo renderPassInfo{};

		renderPassInfo.name = passName + i;
		renderPassInfo.colorLoadOp = first ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD; //TODO: Load previous renderpass target
		renderPassInfo.depthLoadOp = first ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;

		renderPassInfo.AddRenderTarget(renderTarget);
		renderPassInfo.SetDepthStencilTarget(depthTarget);

		passes[i].renderpass = Resources::CreateRenderpass(renderPassInfo, info);
		first = false;

		U32 j = 0;
		for (PipelineInfo& pipeline : info)
		{
			if(pipeline.shader->subpass.inputAttachmentCount) { ++subpass; }
			pipeline.subpass = subpass;
			passes[i].pipelines[j] = Resources::CreatePipeline(pipeline, passes[i].renderpass);
			++j;
		}

		++i;
	}

	ready = true;
}

void PipelineGraph::AddPipeline(const PipelineInfo& info)
{
	ready = false;
	needsRecord = true;
	U32 bestIndex = 0;
	bool found = false;

	U32 i = 0;

	if (info.renderpass >= infos.Size()) { infos.Resize(info.renderpass + 1); }

	for (PipelineInfo& pipeline : infos[info.renderpass])
	{
		if (info.shader->renderOrder < pipeline.shader->renderOrder)
		{
			infos[info.renderpass].Insert(i, info);
			found = true;
			break;
		}

		++i;
	}

	if (!found) { infos[info.renderpass].Push(info); }
}

Texture* PipelineGraph::RenderTarget()
{
	return renderTarget;
}

Texture* PipelineGraph::DepthTarget()
{
	return depthTarget;
}

Pipeline* PipelineGraph::GetPipeline(U32 pass, U32 index)
{
	return passes[pass].pipelines[index];
}

void PipelineGraph::Run(CommandBuffer* commandBuffer)
{
	if (!ready) { Logger::Error("A Rendergraph Must Be Created Before Use!"); return; }

	for (Pass& pass : passes)
	{
		commandBuffer->BeginRenderpass(pass.renderpass);

		U32 subpass = 0;
		U32 i = 0;
		for (Pipeline* pipeline : pass.pipelines)
		{
			if (pipeline->subpass != subpass) { ++subpass; commandBuffer->NextSubpass(); }
			pipeline->Run(commandBuffer);
		}

		commandBuffer->EndRenderpass();
	}

	needsRecord = false;
}

void PipelineGraph::Resize()
{
	Resources::RecreateTexture(renderTarget, Settings::WindowWidth(), Settings::WindowHeight(), 1);
	Resources::RecreateTexture(depthTarget, Settings::WindowWidth(), Settings::WindowHeight(), 1);

	for (Pass& pass : passes)
	{
		if (pass.renderpass->lastResize < Renderer::AbsoluteFrame())
		{
			pass.renderpass->lastResize = Renderer::AbsoluteFrame();

			Resources::RecreateRenderpass(pass.renderpass);
		}
	}

	needsRecord = true;
}