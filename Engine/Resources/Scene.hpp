#pragma once

#include "ResourceDefines.hpp"

#include "ComponentRegistry.hpp"

struct ComponentPool
{
	virtual void Update() = 0;
	virtual void Load(Scene* scene) = 0;

	virtual U64 Count() = 0;
};

template <ComponentType Type>
struct ComponentPoolInternal : public ComponentPool
{
	Type* AddComponent(Type&& component)
	{
		components.Push(Move(component));
		return &components.Back();
	}

	virtual void Update() final
	{
		for (Type& component : components)
		{
			component.Update();
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

struct ComponentReference
{
	U64 type;
	U64 id;
};

struct MeshLocation
{
	U32 instanceOffset{ 0 };
	U32 instanceCount{ 0 };
	U32 drawOffset{ 0 };
};

struct MeshDraw
{
	U32 indexOffset{ 0 };
	U32 indexCount{ 0 };
	I32 vertexOffset{ 0 };

	MeshLocation locations[RENDER_STAGE_COUNT][MAX_PIPELINES_PER_STAGE]{ };
};

struct Entity;
struct MeshInstance;
struct Rendergraph;
struct CommandBuffer;

struct NH_API Scene
{
	Entity* AddEntity();

	const String& Name() { return name; }
	Camera* GetCamera() { return &camera; }

	void AddMesh(MeshInstance& instance);

private:
	void Create(CameraType cameraType, Rendergraph* rendergraph);
	void Destroy();

	template<ComponentType Type, typename... Args>
	Type* AddComponent(ComponentReference& reference, const Args&... args)
	{
		reference.type = IndexOf<RegisteredComponents, Type>;
		reference.id = componentPools[reference.type]->Count();

		return ((ComponentPoolInternal<Type>*)componentPools[reference.type])->AddComponent(Move(Type{ args... }));
	}

	template<ComponentType Type>
	void CreatePool()
	{
		componentPools[IndexOf<RegisteredComponents, Type>] = new ComponentPoolInternal<Type>();
	}

	void Load();
	void Unload();
	void Update();
	bool Render(CommandBuffer* commandBuffer);
	void Resize();

	String							name{};
	HashHandle						handle;
	bool							loaded{ false };

	Camera							camera{};

	Buffer							vertexBuffer;
	Buffer							instanceBuffer;
	Buffer							indexBuffer;
	Buffer							drawBuffer;
	Buffer							countsBuffer;

	U32								vertexOffset{ 0 };
	U32								instanceOffset{ 0 };
	U32								indexOffset{ 0 };
	U32								countsOffset{ 0 };
	U32								drawOffset{ 0 };

	Rendergraph*					rendergraph{ nullptr };

	Hashmap<HashHandle, HashHandle>	handles{ 1024 };
	Vector<MeshDraw>				meshDraws{};
	Vector<Entity>					entities{};

	ComponentPool*					componentPools[RegisteredComponents::Size];

#ifdef NH_DEBUG
	FlyCamera						flyCamera{};
#endif

	friend class Renderer;
	friend class Resources;
	friend struct Entity;
};