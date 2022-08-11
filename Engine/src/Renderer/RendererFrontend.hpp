#pragma once

#include "Defines.hpp"

template<typename> struct Vector;

class RendererFrontend
{
public:
	static bool Initialize(const struct String& applicationName);
	static void Shutdown();

	static bool DrawFrame();
	static bool BeginRenderpass(struct Renderpass* renderpass);
	static bool EndRenderpass(Renderpass* renderpass);

	static bool CreateMesh(struct Mesh* mesh, Vector<struct Vertex>& vertices, Vector<U32>& indices);
	static void DestroyMesh(Mesh* mesh);
	static void DrawMesh(const struct MeshRenderData& Meshdata);

	static void CreateTexture(struct Texture* texture, const Vector<U8>& pixels);
	static void DestroyTexture(Texture* texture);
	static bool CreateWritableTexture(Texture* texture);
	static void WriteTextureData(Texture* texture, const Vector<U8>& pixels);
	static void ResizeTexture(Texture* texture, U32 width, U32 height);

	static bool AcquireTextureMapResources(struct TextureMap& map);
	static void ReleaseTextureMapResources(TextureMap& map);

	static void CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext);
	static void DestroyRenderpass(Renderpass* renderpass);

	static bool CreateRenderTarget(Vector<Texture*>& attachments, struct Renderpass* renderpass, U32 width, U32 height, struct RenderTarget* target);
	static bool DestroyRenderTarget(struct RenderTarget* target, bool freeInternalMemory);
	static Texture* GetWindowAttachment(U8 index);
	static Texture* GetDepthAttachment();
	static U32 GetWindowAttachmentIndex();
	static U8 WindowRenderTargetCount();

	static bool CreateShader(struct Shader* shader);
	static void DestroyShader(Shader* shader);
	static bool InitializeShader(Shader* shader);
	static bool UseShader(Shader* shader);
	static bool ApplyShaderGlobals(Shader* shader);
	static bool ApplyShaderInstance(Shader* shader, bool needsUpdate);
	static U32 AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps);
	static bool ReleaseInstanceResources(Shader* shader, U32 instanceId);
	static bool SetUniform(Shader* shader, struct Uniform& uniform, const void* value);
	static bool SetPushConstant(Shader* shader, struct PushConstant& pushConstant, const void* value);

	static bool OnResize(void* data);
	static NH_API struct Vector2Int WindowSize();
	static NH_API struct Vector2 ScreenToWorld(const struct Vector2Int& v);

	static NH_API void UseScene(class Scene* scene) { activeScene = scene; }

private:
	RendererFrontend() = delete;

	static class Renderer* renderer;

	static bool resizing;
	static U8 framesSinceResize;
	static U8 windowRenderTargetCount;
	static U32 framebufferWidth;
	static U32 framebufferHeight;

	//TODO: Multiple scenes
	static class Scene* activeScene;
};