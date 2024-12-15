#pragma once

#include "ResourceDefines.hpp"

#include "Component.hpp"

struct BufferCopy
{
	U64 srcOffset;
	U64 dstOffset;
	U64 size;
};

struct MeshDraw
{
	U32 indexOffset = 0;
	U32 indexCount = 0;
	I32 vertexOffset = 0;

	U32 instanceOffset = 0;
	U32 instanceCount = 0;
	U32 drawOffset = 0;
};

struct NH_API Entity
{
public:
	Entity(Entity&& other) noexcept;
	Entity& operator=(Entity&& other) noexcept;

	template<ComponentType Type, typename... Args>
	ComponentRef<Type> CreateComponent(Args&&... args) noexcept;

	template<ComponentType Type>
	void DestroyComponent() noexcept;

	template<ComponentType Type>
	ComponentRef<Type> GetComponent() noexcept;

	Transform transform;

private:
	Entity(Scene* scene, U32 id) : scene(scene), entityID(id) {}

	Scene* scene;
	U32 entityID;
	Vector<U32> references;

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	friend struct Scene;
};

struct Mesh;
struct Entity;
struct Pipeline;
struct MeshInstance;
struct CommandBuffer;

struct NH_API Scene
{
	Entity* CreateEntity();
	Entity* GetEntity(U32 id);

	const String& Name() { return name; }
	const Camera& GetCamera()
	{
#ifdef NH_DEBUG
		return flyCamera.GetCamera();
#else
		return camera;
#endif
	}

	void SetSkybox(const ResourceRef<Skybox>& skybox);
	void SetPostProcessing(const PostProcessData& data);
	void AddPipeline(ResourceRef<Pipeline>& pipeline);

	void AddInstance(MeshInstance& instance);
	void UpdateInstance(MeshInstance& instance);

	void Destroy();

private:
	void Create(CameraType cameraType);

	void Load();
	void Unload();
	void Update();
	void Render(CommandBuffer* commandBuffer);
	void Resize();

	void AddMesh(ResourceRef<Mesh>& mesh);
	BufferCopy CreateWrite(U64 dstOffset, U64 srcOffset, U64 size, const void* data);

	String							name;
	HashHandle						handle;
	bool							loaded = false;
	bool							hasSkybox = false;
	bool							hasPostProcessing = false;

	Buffer							stagingBuffer;
	Buffer							entitiesBuffer;
	Buffer							vertexBuffers[VERTEX_TYPE_COUNT - 1];
	Buffer							instanceBuffer;
	Buffer							indexBuffer;
	Buffer							drawBuffer;
	Buffer							countsBuffer;

	Vector<BufferCopy>				entityWrites;
	Vector<BufferCopy>				vertexWrites[VERTEX_TYPE_COUNT - 1];
	Vector<BufferCopy>				instanceWrites;
	Vector<BufferCopy>				indexWrites;
	Vector<BufferCopy>				drawWrites;
	Vector<BufferCopy>				countsWrites;

	U32								vertexOffset = 0;
	U32								instanceOffset = 0;
	U32								indexOffset = 0;
	U32								drawOffset = 0;
	U32								countsOffset = 0;

	Vector<MeshDraw>				meshDraws;
	Vector<Entity>					entities; //TODO: Maybe store all transforms in a separate array for faster buffer copy

	Vector<ResourceRef<Pipeline>>	pipelines;
	Vector<Renderpass>				renderpasses;

#ifdef NH_DEBUG
	FlyCamera						flyCamera;
#else
	Camera							camera;
#endif

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	friend class Renderer;
	friend class Resources;
	friend struct Entity;
};

template<ComponentType Type, typename... Args>
inline ComponentRef<Type> Entity::CreateComponent(Args&&... args) noexcept
{
	Type* type;
	ComponentRef<Type> ref = ComponentRegistry::CreateFunction<Type>()(scene, entityID, &type);

	Construct(type, Type{ args... });
	type->Load(scene);
	return ref;
}

template<ComponentType Type>
void Entity::DestroyComponent() noexcept
{
	ComponentRegistry::DestroyFunction<Type>()(scene, entityID);
}

template<ComponentType Type>
inline ComponentRef<Type> Entity::GetComponent() noexcept
{
	return ComponentRegistry::GetFunction<Type>()(entityID);
}