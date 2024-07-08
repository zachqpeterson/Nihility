#pragma once

#include "ResourceDefines.hpp"

#include "Component.hpp"

struct ComponentPool
{
	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;
	virtual void Cleanup(Scene* scene) = 0;

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

	virtual void Cleanup(Scene* scene) final
	{
		for (Type& component : components)
		{
			component.Cleanup(scene);
		}
	}

	virtual U64 Count() final { return components.Size(); }

	Vector<Type> components; //TODO: Use freelist and array
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

	template<ComponentType Type, typename... Args>
	void RegisterComponent() noexcept
	{
		StringView sv = NameOf<Type>;

		U32* id = componentRegistry.Request(sv);

		if (*id == 0)
		{
			*id = currentId++;
			componentPools.Push(new ComponentPoolInternal<Type>());
		}
	}

	template<ComponentType Type>
	Vector<Type>* GetComponentPool() noexcept
	{
		U32* id = componentRegistry.Request(NameOf<Type>);

		if (*id == 0) { return nullptr; }

		return &((ComponentPoolInternal<Type>*)componentPools[*id - 1])->components;
	}

private:
	void Create(CameraType cameraType);

	template<ComponentType Type, typename... Args>
	Type* AddComponent(U32 entityID, ComponentReference& reference, Args&&... args) noexcept
	{
		StringView sv = NameOf<Type>;

		U32* id = componentRegistry.Request(sv);

		if (*id == 0)
		{
			*id = currentId++;
			componentPools.Push(new ComponentPoolInternal<Type>());
		}

		reference.type = *id - 1;
		reference.id = componentPools[reference.type]->Count();

		Type* component = ((ComponentPoolInternal<Type>*)componentPools[reference.type])->AddComponent(Move(Type{ args... }));
		component->entityID = entityID;
		if (loaded) { component->Load(this); }

		return component;
	}

	template<ComponentType Type>
	Type* GetComponent(const Vector<ComponentReference>& references) noexcept
	{
		U32* id = componentRegistry.Request(NameOf<Type>);
		if (*id == 0) { return nullptr; }

		U32 i = *id - 1;

		for (ComponentReference& ref : references)
		{
			if (ref.id == i) { return ((ComponentPoolInternal<Type>*)componentPools[i])->components[ref.id]; }
		}

		return nullptr;
	}

	template<ComponentType Type>
	Vector<Type*> GetComponents(const Vector<ComponentReference>& references) noexcept
	{
		U32* id = componentRegistry.Request(NameOf<Type>);
		if (*id == 0) { return Move(Vector<Type*>()); }

		U32 i = *id - 1;
		Vector<Type*> types;

		ComponentPoolInternal<Type>* pool = ((ComponentPoolInternal<Type>*)componentPools[i]);

		for (ComponentReference& ref : references)
		{
			if (ref.id == i) { types.Push(pool->components[ref.id]); }
		}

		return Move(types);
	}

	void Load();
	void Unload();
	void Update();
	void Render(CommandBuffer* commandBuffer);
	void Resize();

	void AddMesh(ResourceRef<Mesh>& mesh);
	BufferCopy CreateWrite(U64 dstOffset, U64 srcOffset, U64 size, const void* data);

	String							name{};
	HashHandle						handle;
	bool							loaded{ false };
	bool							hasSkybox{ false };
	bool							hasPostProcessing{ false };

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

	U32								vertexOffset{ 0 };
	U32								instanceOffset{ 0 };
	U32								indexOffset{ 0 };
	U32								drawOffset{ 0 };
	U32								countsOffset{ 0 };

	Vector<MeshDraw>				meshDraws;
	Vector<Entity>					entities; //TODO: Maybe store all transforms in a separate array for faster buffer copy

	U32								currentId{ 1 };
	Hashmap<StringView, U32>		componentRegistry{ 32 };
	Vector<ComponentPool*>			componentPools;
	Vector<ResourceRef<Pipeline>>	pipelines;
	Vector<Renderpass>				renderpasses;

#ifdef NH_DEBUG
	FlyCamera						flyCamera{};
#else
	Camera							camera{};
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
		Type* component = scene->AddComponent<Type>(entityID, reference, args...);
		references.Push(reference);

		return component;
	}

	template<ComponentType Type>
	Type* GetComponent() noexcept
	{
		return scene->GetComponent<Type>(references);
	}

	template<ComponentType Type>
	Vector<Type*> GetComponents() noexcept
	{
		return Move(scene->GetComponents<Type>(references));
	}

	Transform transform{};

private:
	Entity(Scene* scene, U32 id) : scene{ scene }, entityID{ id } {}

	Scene* scene;
	U32 entityID;
	Vector<ComponentReference> references;

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	friend struct Scene;
};