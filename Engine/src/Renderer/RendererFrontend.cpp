#include "RendererFrontend.hpp"

#include "RendererDefines.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

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
U8 RendererFrontend::framesSinceResize;
U8 RendererFrontend::windowRenderTargetCount;
U32 RendererFrontend::framebufferWidth;
U32 RendererFrontend::framebufferHeight;

Scene* RendererFrontend::activeScene;

bool RendererFrontend::Initialize(const String& applicationName)
{
	Events::Subscribe("Resize", OnResize);
	framebufferWidth = Settings::WindowWidth;
	framebufferHeight = Settings::WindowHeight;

	resizing = false;
	framesSinceResize = 0;

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

	if (resizing)
	{
		if (++framesSinceResize >= 30)
		{
			renderer->OnResize();
			activeScene->OnResize();

			framesSinceResize = 0;
			resizing = false;
		}
		else
		{
			return true;
		}
	}

	if (renderer->BeginFrame())
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

bool RendererFrontend::CreateMesh(Mesh* mesh, Vector<Vertex>& vertices, Vector<U32>& indices)
{
	return renderer->CreateMesh(mesh, vertices, indices);
}

void RendererFrontend::DestroyMesh(Mesh* mesh)
{
	renderer->DestroyMesh(mesh);
}

void RendererFrontend::DrawMesh(const struct MeshRenderData& Meshdata)
{
	renderer->DrawMesh(Meshdata);
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

void RendererFrontend::CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext)
{
	renderer->CreateRenderpass(renderpass, hasPrev, hasNext);
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

bool RendererFrontend::UseShader(Shader* shader)
{
	return renderer->UseShader(shader);
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

bool RendererFrontend::SetUniform(Shader* shader, Uniform& uniform, const void* value)
{
	return renderer->SetUniform(shader, uniform, value);
}

bool RendererFrontend::SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value)
{
	return renderer->SetPushConstant(shader, pushConstant, value);
}

bool RendererFrontend::OnResize(void* data)
{
	framebufferWidth = Settings::WindowWidth;
	framebufferHeight = Settings::WindowHeight;
	framesSinceResize = 0;
	resizing = true;

	return true;
}

Vector2Int RendererFrontend::WindowSize()
{
	return { (I32)framebufferWidth, (I32)framebufferHeight };
}