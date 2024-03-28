#pragma once

#include "ResourceDefines.hpp"

#include "Component.hpp"
#include "Entity.hpp"

struct ComponentPool
{
	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;

	virtual U64 Count() = 0;
};

struct BufferCopy
{
	U64 srcOffset;
	U64 dstOffset;
	U64 size;
};

template <class Type>
struct ComponentPoolInternal : public ComponentPool
{
	Type* AddComponent(Type&& component)
	{
		components.Push(Move(component));
		return &components.Back();
	}

	virtual void Update(Scene* scene) final
	{
		for (Type& component : components)
		{
			component.Update(scene);
		}
	}

	virtual void Load(Scene* scene) final
	{
		for (Type& component : components)
		{
			component.Load(scene);
		}
	}

	virtual U64 Count() final { return components.Size(); }

	Vector<Type> components;
};

struct MeshDraw
{
	U32 indexOffset{ 0 };
	U32 indexCount{ 0 };
	I32 vertexOffset{ 0 };

	U32 instanceOffset{ 0 };
	U32 instanceCount{ 0 };
	U32 drawOffset{ 0 };
};

struct ComponentReference
{
	U64 type;
	U64 id;
};

struct Mesh;
struct Entity;
struct Pipeline;
struct MeshInstance;
struct CommandBuffer;

struct NH_API Scene
{
	Entity* AddEntity();
	Entity* GetEntity(U32 id);

	const String& Name() { return name; }
	Camera* GetCamera() { return &camera; }

	void SetSkybox(const ResourceRef<Skybox>& skybox);
	void SetPostProcessing(const PostProcessData& data);

	void AddInstance(MeshInstance& instance);
	void UpdateInstance(MeshInstance& instance);

	void Destroy();

private:
	void Create(CameraType cameraType);

	template<ComponentType Type, typename... Args>
	Type* AddComponent(ComponentReference& reference, Args&&... args) noexcept
	{
		U32* id = componentRegistry.Request(NameOf<Type>);

		if (*id == 0)
		{
			*id = currentId++;
			componentPools.Push(new ComponentPoolInternal<Type>());
		}

		reference.type = *id - 1;
		reference.id = componentPools[reference.type]->Count();

		Type* component = ((ComponentPoolInternal<Type>*)componentPools[reference.type])->AddComponent(Move(Type{ args... }));

		if (loaded) { component->Load(this); }

		return component;
	}

	void Load();
	void Unload();
	void Update();
	void Render(CommandBuffer* commandBuffer);
	void Resize();

	void AddMesh(ResourceRef<Mesh>& mesh);
	void AddPipeline(ResourceRef<Pipeline>& pipeline);
	BufferCopy CreateWrite(U64 dstOffset, U64 srcOffset, U64 size, void* data);

	String							name{};
	HashHandle						handle;
	bool							loaded{ false };
	bool							hasSkybox{ false };
	bool							hasPostProcessing{ false };

	Camera							camera{};

	Buffer							stagingBuffer;
	Buffer							vertexBuffers[VERTEX_TYPE_COUNT - 1];
	Buffer							instanceBuffer;
	Buffer							indexBuffer;
	Buffer							drawBuffer;
	Buffer							countsBuffer;

	Vector<BufferCopy>				vertexWrites[VERTEX_TYPE_COUNT - 1];
	Vector<BufferCopy>				indexWrites;
	Vector<BufferCopy>				drawWrites;
	Vector<BufferCopy>				countsWrites;

	U32								vertexOffset{ 0 };
	U32								instanceOffset{ 0 };
	U32								indexOffset{ 0 };
	U32								drawOffset{ 0 };
	U32								countsOffset{ 0 };

	Vector<MeshDraw>				meshDraws{};
	Vector<Entity>					entities{};

	U32								currentId{ 1 };
	Hashmap<StringView, U32>		componentRegistry{ 32 };
	Vector<ComponentPool*>			componentPools{};
	Vector<ResourceRef<Pipeline>>	pipelines{};
	Vector<Renderpass>				renderpasses{};

#ifdef NH_DEBUG
	FlyCamera						flyCamera{};
#endif

	friend class Renderer;
	friend class Resources;
	friend struct Entity;
};

struct NH_API Entity
{
public:
	Entity(Entity&& other) noexcept : transform{ other.transform }, scene{ other.scene }, entityID{ other.entityID }, references{ Move(references) } {}
	Entity& operator=(Entity&& other) noexcept
	{
		transform = other.transform;
		scene = other.scene;
		entityID = other.entityID;
		references = Move(references);

		return *this;
	}

	template<ComponentType Type, typename... Args>
	Type* AddComponent(Args&&... args) noexcept
	{
		ComponentReference reference;
		Type* component = scene->AddComponent<Type>(reference, args...);
		component->entityID = entityID;
		references.Push(reference);

		return component;
	}

	Transform transform{};

private:
	Entity(Scene* scene, U32 id) : scene{ scene }, entityID{ id } {}

	Scene* scene;
	U32 entityID;
	Vector<ComponentReference> references{};

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	friend struct Scene;
};