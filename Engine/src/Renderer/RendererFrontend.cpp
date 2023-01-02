#include "RendererFrontend.hpp"

#include "RendererDefines.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Core/Events.hpp"
#include "Core/Settings.hpp"
#include "Memory/Memory.hpp"
#include "Containers/String.hpp"
#include <Containers/Vector.hpp>
#include "Vulkan/VulkanRenderer.hpp"
#include "Resources/Resources.hpp"
#include "Math/Math.hpp"

Renderer* RendererFrontend::renderer;

bool RendererFrontend::resizing;
U8 RendererFrontend::windowRenderTargetCount;
U32 RendererFrontend::framebufferWidth;
U32 RendererFrontend::framebufferHeight;

Scene* RendererFrontend::activeScene;

bool RendererFrontend::Initialize(const String& applicationName)
{
	framebufferWidth = Settings::WindowWidth;
	framebufferHeight = Settings::WindowHeight;

	resizing = false;

	//Try vulkan
	renderer = new VulkanRenderer();
	if (!renderer->Initialize(applicationName, windowRenderTargetCount))
	{
		delete renderer;
		Logger::Error("Vulkan isn't supported on this machine!");

		//If windows, try DirectX

		//Try OpenGL

		return false;
	}

	return true;
}

void RendererFrontend::Shutdown()
{
	renderer->Shutdown();
	delete renderer;
}

bool RendererFrontend::DrawFrame()
{
	++renderer->frameNumber;

	if (resizing && !Settings::Minimised)
	{
		renderer->OnResize();
		resizing = false;
		Events::Notify("Resize", NULL);
	}

	if (!resizing && renderer->BeginFrame())
	{
		U8 attachmentIndex = renderer->GetWindowAttachmentIndex();

		activeScene->OnRender(renderer->frameNumber, attachmentIndex);

		if (!renderer->EndFrame())
		{
			Logger::Error("End frame failed. Application shutting down...");
			return false;
		}
	}

	return true;
}

bool RendererFrontend::BeginRenderpass(Renderpass* renderpass)
{
	return renderer->BeginRenderpass(renderpass);
}

bool RendererFrontend::EndRenderpass(Renderpass* renderpass)
{
	return renderer->EndRenderpass(renderpass);
}

bool RendererFrontend::CreateMesh(Mesh* mesh)
{
	return renderer->CreateMesh(mesh);
}

bool RendererFrontend::BatchCreateMeshes(Vector<Mesh*>& meshes)
{
	return renderer->BatchCreateMeshes(meshes);
}

void RendererFrontend::DestroyMesh(Mesh* mesh)
{
	renderer->DestroyMesh(mesh);
}

void RendererFrontend::DrawMesh(const struct MeshRenderData& meshData)
{
	renderer->DrawMesh(meshData);
}

void RendererFrontend::CreateTexture(Texture* texture, const Vector<U8>& pixels)
{
	renderer->CreateTexture(texture, pixels);
}

void RendererFrontend::DestroyTexture(Texture* texture)
{
	renderer->DestroyTexture(texture);
}

bool RendererFrontend::CreateWritableTexture(Texture* texture)
{
	return renderer->CreateWritableTexture(texture);
}

void RendererFrontend::WriteTextureData(Texture* texture, const Vector<U8>& pixels)
{
	renderer->WriteTextureData(texture, pixels);
}

void RendererFrontend::ResizeTexture(Texture* texture, U32 width, U32 height)
{
	renderer->ResizeTexture(texture, width, height);
}

bool RendererFrontend::AcquireTextureMapResources(TextureMap& map)
{
	return renderer->AcquireTextureMapResources(map);
}

void RendererFrontend::ReleaseTextureMapResources(TextureMap& map)
{
	renderer->ReleaseTextureMapResources(map);
}

void RendererFrontend::CreateRenderpass(Renderpass* renderpass)
{
	renderer->CreateRenderpass(renderpass);
}

void RendererFrontend::DestroyRenderpass(Renderpass* renderpass)
{
	renderer->DestroyRenderpass(renderpass);
}

bool RendererFrontend::CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, RenderTarget* target)
{
	return renderer->CreateRenderTarget(attachments, renderpass, width, height, target);
}

bool RendererFrontend::DestroyRenderTarget(RenderTarget* target, bool freeInternalMemory)
{
	return renderer->DestroyRenderTarget(target, freeInternalMemory);
}

Texture* RendererFrontend::GetWindowAttachment(U8 index)
{
	return renderer->GetWindowAttachment(index);
}

Texture* RendererFrontend::GetDepthAttachment()
{
	return renderer->GetDepthAttachment();
}

U32 RendererFrontend::GetWindowAttachmentIndex()
{
	return renderer->GetWindowAttachmentIndex();
}

U8 RendererFrontend::WindowRenderTargetCount()
{
	return windowRenderTargetCount;
}

bool RendererFrontend::CreateShader(Shader* shader)
{
	return renderer->CreateShader(shader);
}

void RendererFrontend::DestroyShader(Shader* shader)
{
	renderer->DestroyShader(shader);
}

bool RendererFrontend::InitializeShader(Shader* shader)
{
	return renderer->InitializeShader(shader);
}

void RendererFrontend::UseShader(Shader* shader)
{
	renderer->UseShader(shader);
}

bool RendererFrontend::ApplyShaderGlobals(Shader* shader)
{
	return renderer->ApplyGlobals(shader);
}

bool RendererFrontend::ApplyShaderInstance(Shader* shader, bool needsUpdate)
{
	return renderer->ApplyInstance(shader, needsUpdate);
}

U32 RendererFrontend::AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps)
{
	return renderer->AcquireInstanceResources(shader, maps);
}

bool RendererFrontend::ReleaseInstanceResources(Shader* shader, U32 instanceId)
{
	return renderer->ReleaseInstanceResources(shader, instanceId);
}

void RendererFrontend::SetGlobalUniform(Shader* shader, Uniform& uniform, const void* value)
{
	return renderer->SetGlobalUniform(shader, uniform, value);
}

void RendererFrontend::SetInstanceUniform(Shader* shader, Uniform& uniform, const void* value)
{
	return renderer->SetInstanceUniform(shader, uniform, value);
}

void RendererFrontend::SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value)
{
	return renderer->SetPushConstant(shader, pushConstant, value);
}

void RendererFrontend::OnResize()
{
	framebufferWidth = Settings::WindowWidth;
	framebufferHeight = Settings::WindowHeight;
	resizing = true;
}

Vector2Int RendererFrontend::WindowSize()
{
	return renderer->WindowSize();
}

Vector2Int RendererFrontend::WindowOffset()
{
	return renderer->WindowOffset();
}

Vector2 RendererFrontend::ScreenToWorld(const Vector2& v)
{
	Vector3 camPos = activeScene->GetCamera()->Position();
	
	return { camPos.x + (F32)(v.x - (I32)framebufferWidth / 2) * 0.02857142857f, camPos.y + (F32)(v.y - (I32)framebufferHeight / 2) * 0.02857142857f };
}

void RendererFrontend::DrawGameObject(GameObject2D* go)
{
	activeScene->DrawGameObject(go);
}

void RendererFrontend::UndrawGameObject(GameObject2D* go)
{
	activeScene->UndrawGameObject(go);
}

void RendererFrontend::DrawModel(Model* model)
{
	activeScene->DrawModel(model);
}

void RendererFrontend::UndrawModel(Model* model)
{
	activeScene->UndrawModel(model);
}