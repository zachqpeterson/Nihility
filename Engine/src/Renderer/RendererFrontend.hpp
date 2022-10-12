#pragma once

#include "Defines.hpp"

template<typename> struct Vector;
struct Mesh;
struct Renderpass;
struct String;
struct MeshRenderData;
struct Texture;
struct TextureMap;
struct RenderTarget;
struct Shader;
struct Uniform;
struct PushConstant;
struct Vector2;
struct Vector2Int;
struct GameObject2D;
struct Model;
class Scene;

class RendererFrontend
{
public:
	static bool Initialize(const String& applicationName);
	static void Shutdown();

	static bool DrawFrame();
	static bool BeginRenderpass(Renderpass* renderpass);
	static bool EndRenderpass(Renderpass* renderpass);

	static NH_API bool CreateMesh(Mesh* mesh);
	static NH_API bool BatchCreateMeshes(Vector<Mesh*>& meshes);
	static NH_API void DestroyMesh(Mesh* mesh);
	static void DrawMesh(const MeshRenderData& meshData);

	static void CreateTexture(Texture* texture, const Vector<U8>& pixels);
	static void DestroyTexture(Texture* texture);
	static bool CreateWritableTexture(Texture* texture);
	static void WriteTextureData(Texture* texture, const Vector<U8>& pixels);
	static void ResizeTexture(Texture* texture, U32 width, U32 height);

	static bool AcquireTextureMapResources(TextureMap& map);
	static void ReleaseTextureMapResources(TextureMap& map);

	static void CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext);
	static void DestroyRenderpass(Renderpass* renderpass);

	static bool CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, RenderTarget* target);
	static bool DestroyRenderTarget(RenderTarget* target, bool freeInternalMemory);
	static Texture* GetWindowAttachment(U8 index);
	static Texture* GetDepthAttachment();
	static U32 GetWindowAttachmentIndex();
	static U8 WindowRenderTargetCount();

	static bool CreateShader(Shader* shader);
	static void DestroyShader(Shader* shader);
	static bool InitializeShader(Shader* shader);
	static void UseShader(Shader* shader);
	static bool ApplyShaderGlobals(Shader* shader);
	static bool ApplyShaderInstance(Shader* shader, bool needsUpdate);
	static U32 AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps);
	static bool ReleaseInstanceResources(Shader* shader, U32 instanceId);
	static void SetGlobalUniform(Shader* shader, Uniform& uniform, const void* value);
	static void SetInstanceUniform(Shader* shader, Uniform& uniform, const void* value);
	static void SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value);

	static bool OnResize(void* data);
	static NH_API Vector2Int WindowSize();
	static NH_API Vector2Int WindowOffset();
	static NH_API Vector2 ScreenToWorld(const Vector2& v);

	static NH_API void DrawGameObject(GameObject2D* go);
	static NH_API void UndrawGameObject(GameObject2D* go);
	static NH_API void DrawModel(Model* model);
	static NH_API void UndrawModel(Model* model);
	static NH_API void UseScene(Scene* scene) { activeScene = scene; } //TODO: scene change event
	static NH_API Scene* CurrentScene() { return activeScene; }

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