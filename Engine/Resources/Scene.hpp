#pragma once

#include "ResourceDefines.hpp"

import Core;
import Memory;

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

struct Scene;

template<class Type>
struct NH_API Component
{
	U32 GetEntityID() const { return entityID; }

protected:
	Component() {}
	Component(const Component& other) noexcept : entityID(other.entityID) {}
	Component(Component&& other) noexcept : entityID(other.entityID) {}
	Component& operator=(Component&& other) noexcept { entityID = other.entityID; return *this; }

	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;
	virtual void Cleanup(Scene* scene) = 0;

	U32 entityID = U32_MAX;

private:
	static inline U32 index = U32_MAX;

	friend struct Scene;
	friend struct Entity;
	friend struct ComponentRegistry;

	template<class Type>
	struct ComponentFactory;
	friend struct ComponentFactory<Type>;
};

template <class Type> constexpr const bool IsComponent = InheritsFrom<Type, Component<Type>>;
template <class Type> concept ComponentType = IsComponent<Type>;

template<ComponentType Type>
struct NH_API ComponentRef
{
public:
	ComponentRef() {}
	ComponentRef(U32 index, Vector<Type>* array) : index(index), array(array) {}
	ComponentRef(NullPointer) {}
	ComponentRef(const ComponentRef& other) : index(other.index), array(other.array) {}
	~ComponentRef() { index = U32_MAX; array = nullptr; }

	ComponentRef& operator=(NullPointer) { index = U32_MAX; array = nullptr; return *this; }
	ComponentRef& operator=(const ComponentRef& other) { index = other.index; array = other.array; return *this; }

	bool operator==(const ComponentRef<Type>& other) const { return index == other.index; }
	bool operator!=(const ComponentRef<Type>& other) const { return index != other.index; }

	Type* operator->();
	const Type* operator->() const;

	Type& operator*();
	const Type& operator*() const;

	operator bool() const { return index != U32_MAX; }

private:
	U32 index = U32_MAX;
	Vector<Type>* array = nullptr;
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

template<ComponentType Type>
struct NH_API ComponentFactory
{
protected:
	virtual ComponentRef<Type> CreateComponent(Scene* scene, U32 entityID, Type** type)
	{
		U32 index = freelist.GetFree();

		if (index == components.Size()) { components.PushEmpty(); }

		*type = &components[index];

		return ComponentRef<Type>(index, &components);
	}

	virtual void DestroyComponent(Scene* scene, U32 entityID)
	{
		for (Type* it = components.begin(); it != components.end(); ++it)
		{
			if (it->GetEntityID() == entityID)
			{
				freelist.Release((U32)(it - components.begin()));

				it->Cleanup(scene);
			}
		}
	}

	virtual ComponentRef<Type> GetComponent(U32 entityID)
	{
		for (Type* it = components.begin(); it != components.end(); ++it)
		{
			if (it->GetEntityID() == entityID)
			{
				return ComponentRef<Type>((U32)(it - components.begin()), &components);
			}
		}

		return nullptr;
	}

public:
	virtual I32 GetComponentID(Type& component)
	{
		return (I32)components.Index(&component);
	}

	Freelist freelist = 256;
	Vector<Type> components;

	friend struct Entity;
	friend struct ComponentRef<Type>;
	friend struct ComponentRegistry;
};

template<ComponentType Type>
inline Type* ComponentRef<Type>::operator->() { return &array->Get(index); }

template<ComponentType Type>
inline const Type* ComponentRef<Type>::operator->() const { return &array->Get(index); }

template<ComponentType Type>
inline Type& ComponentRef<Type>::operator*() { return array->Get(index); }

template<ComponentType Type>
inline const Type& ComponentRef<Type>::operator*() const { return array->Get(index); }

template <ComponentType Type>
using ComponentCreateFn = Function<ComponentRef<Type>(Scene*, U32, Type**)>;
template <ComponentType Type>
using ComponentDestroyFn = Function<void(Scene*, U32)>;
template <ComponentType Type>
using ComponentGetFn = Function<ComponentRef<Type>(U32)>;

struct NH_API ComponentRegistry
{
private:
	template<ComponentType Type>
	struct FactoryStorage
	{
		ComponentFactory<Type> factory;

		ComponentCreateFn<Type> CreateFunction;
		ComponentDestroyFn<Type> DestroyFunction;
		ComponentGetFn<Type> GetFunction;
	};

public:
	template<ComponentType Type>
	static void RegisterComponent(const ComponentCreateFn<Type>& createFn, const ComponentDestroyFn<Type>& destroyFn, const ComponentGetFn<Type>& getFn)
	{
		if (Type::index == U32_MAX)
		{
			Type::index = (U32)components.Size();
			FactoryStorage<Type>* storage = (FactoryStorage<Type>*)components.Push(nullptr);

			Memory::Allocate(&storage);
			storage->CreateFunction = createFn;
			storage->DestroyFunction = destroyFn;
			storage->GetFunction = getFn;
		}
	}

	template<ComponentType Type>
	static void RegisterComponent()
	{
		if (Type::index == U32_MAX)
		{
			Type::index = (U32)components.Size();
			FactoryStorage<Type>* storage = (FactoryStorage<Type>*)components.Push(nullptr);

			Memory::Allocate(&storage);
			storage->factory = {};
			storage->CreateFunction = Bind(&ComponentFactory<Type>::CreateComponent, &storage->factory, Placeholder1, Placeholder2, Placeholder3);
			storage->DestroyFunction = Bind(&ComponentFactory<Type>::DestroyComponent, &storage->factory, Placeholder1, Placeholder2);
			storage->GetFunction = Bind(&ComponentFactory<Type>::GetComponent, &storage->factory, Placeholder1);
		}
	}

	template<ComponentType Type>
	static ComponentCreateFn<Type>& CreateFunction()
	{
		if (Type::index == U32_MAX) { RegisterComponent<Type>(); }

		return ((FactoryStorage<Type>*)components[Type::index])->CreateFunction;
	}

	template<ComponentType Type>
	static ComponentDestroyFn<Type>& DestroyFunction()
	{
		if (Type::index == U32_MAX) { RegisterComponent<Type>(); }

		return ((FactoryStorage<Type>*)components[Type::index])->DestroyFunction;
	}

	template<ComponentType Type>
	static ComponentGetFn<Type>& GetFunction()
	{
		if (Type::index == U32_MAX) { RegisterComponent<Type>(); }

		return ((FactoryStorage<Type>*)components[Type::index])->GetFunction;
	}

	static Vector<void*> components;
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