#pragma once

#include "ResourceDefines.hpp"
#include "Introspection.hpp"

#include "Component.hpp"

#include "Containers\Bitset.hpp"
#include "Containers\Array.hpp"
#include "Containers\Vector.hpp"
#include "Core\Function.hpp"
#include "Core\Logger.hpp"

#include <tuple>

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

struct SparseSetBase
{
	virtual ~SparseSetBase() = default;
	virtual void Remove(U32) = 0;
	virtual void Clear() = 0;
	virtual U32 Size() = 0;
	virtual bool ContainsIndex(U32) = 0;
	virtual Vector<U32> IndexList() = 0;
	virtual void Load(Scene* scene) = 0;
};

template<class Type>
struct SparseSet : public SparseSetBase
{
private:
	static constexpr U32 PageSize = 256;
	static constexpr U32 Invalid = U32_MAX;

	using Sparse = Array<U32, PageSize>;

public:
	SparseSet();

	template<typename... Args>
	Type* Emplace(U32 id, Args&&... args);
	Type* Add(U32 id, Type&& obj);
	Type* Get(U32 id);
	Type& GetRef(U32 id);
	void Remove(U32 id) override;

	U32 Size() override;
	Vector<U32> IndexList() override;
	bool ContainsIndex(U32 id) override;
	void Clear() override;
	void Load(Scene* scene) override;
	bool IsEmpty() const;
	const Vector<Type>& Data() const;

private:
	void SetDenseIndex(U32 id, U32 index);
	U32 GetDenseIndex(U32 id);

	Vector<Sparse> pages;
	Vector<Type> dense;
	Vector<U32> denseToEntity;

	SparseSet(const SparseSet&) = delete;
	SparseSet(SparseSet&&) = delete;
	SparseSet& operator=(const SparseSet&) = delete;
	SparseSet& operator=(SparseSet&&) = delete;
};

template <class... Types>
struct TypeList {
	using Tuple = std::tuple<Types...>;

	template <U64 Index>
	using Get = std::tuple_element_t<Index, Tuple>;

	static constexpr U64 size = sizeof...(Types);
};

template<typename... Types>
struct View
{
private:
	using types = TypeList<Types...>;

	bool AllContain(U32 id);

	template <U64 Index>
	auto GetPoolAt();

	template <U64... Indices>
	auto MakeComponentTuple(U32 id, IndexSequence<Indices...>);

	template <typename Func>
	void ForEachImpl(Func func);

public:
	using ForEachFunc = Function<void(Types&...)>;
	using ForEachFuncWithID = Function<void(U32, Types&...)>;

	View(Array<SparseSetBase*, sizeof...(Types)> pools);

	void ForEach(ForEachFunc func);
	void ForEach(ForEachFuncWithID func);

	struct Pack {
		U32 id;
		std::tuple<Types&...> components;
	};

	Vector<Pack> GetPacked();

private:
	Array<SparseSetBase*, sizeof...(Types)> viewPools;

	SparseSetBase* smallest = nullptr;
};

struct NH_API Entity
{
public:
	Entity();

	template<class Type, typename... Args>
	Type* AddComponent(Args&&... args) noexcept;

	template<class Type>
	Type* GetComponent();

	template <class Type>
	void RemoveComponent();

private:
	Entity(U32 id, Scene* scene);

	U32 id;
	Scene* scene;

	friend Scene;
};

struct Mesh;
struct Entity;
struct Pipeline;
struct MeshInstance;
struct CommandBuffer;

struct NH_API Scene
{
private:
	static constexpr U32 Invalid = U32_MAX;

	static constexpr U64 MaxComponents = 64;

	template<class Type>
	U64 GetComponentBitPosition();

	template<class Type>
	SparseSetBase* GetComponentPoolPtr();

	template <class Type>
	SparseSet<Type>& GetComponentPool();

	template <class Type>
	void SetComponentBit(Bitset& mask, bool val);

	template <class Type>
	void AddComponentBit();

	Bitset& GetEntityMask(U32 id);

	template <class... Types>
	Bitset GetMask();

	template <class Type>
	void RegisterComponent();

	template <class Type, typename... Args>
	Type* AddComponent(U32 id, Args&&... args);

	template <class Type>
	Type* GetComponent(U32 id);

	template <class Type>
	void DestroyComponent(U32 id);

public:
	Entity CreateEntity();
	void DestroyEntity(Entity& id);

	template<class Type>
	void LoadComponents();

	template <class... Types>
	View<Types...> GetView();

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

	Vector<U32> availableEntities;
	SparseSet<Bitset> entityMasks;
	Vector<SparseSetBase*> componentPools;
	Hashmap<StringView, U64> componentBitPosition;
	U32 maxEntityID = 0;

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

template <class Type, typename... Args>
inline Type* Scene::AddComponent(U32 id, Args&&... args)
{
	SparseSet<Type>& pool = GetComponentPool<Type>();

	if (pool.Get(id)) { return pool.Emplace(id, Forward<Args>(args)...); }

	Bitset& mask = GetEntityMask(id);

	SetComponentBit<Type>(mask, 1);

	Type* t = pool.Emplace(id, Forward<Args>(args)...);

	if (loaded) { t->Load(this, id); }

	return t;
}

template <class Type>
inline Type* Scene::GetComponent(U32 id)
{
	SparseSet<Type>& pool = GetComponentPool<Type>();

	return pool.Get(id);
}

template <class Type>
inline void Scene::DestroyComponent(U32 id)
{
	SparseSet<Type>& pool = GetComponentPool<Type>();

	if (!pool.Get(id)) { return; }

	//TODO: call cleanup

	Bitset& mask = GetEntityMask(id);
	SetComponentBit<Type>(mask, 0);

	pool.Remove(id);
}

template <class Type>
inline void Scene::RegisterComponent()
{
	AddComponentBit<Type>();

	componentPools.Push(new SparseSet<Type>());
}

template<class Type>
inline U64 Scene::GetComponentBitPosition()
{
	U64* index = componentBitPosition.Get(NameOf<Type>);

	if (index) { return *index; }

	return Invalid;
}

template<class Type>
inline SparseSetBase* Scene::GetComponentPoolPtr()
{
	U64 bitPos = GetComponentBitPosition<Type>();

	if (bitPos == Invalid)
	{
		RegisterComponent<Type>();
		bitPos = GetComponentBitPosition<Type>();
	}

	return componentPools[bitPos];
}

template <class Type>
inline SparseSet<Type>& Scene::GetComponentPool()
{
	SparseSetBase* genericPtr = GetComponentPoolPtr<Type>();
	SparseSet<Type>* pool = static_cast<SparseSet<Type>*>(genericPtr);

	return *pool;
}

template <class Type>
inline void Scene::SetComponentBit(Bitset& mask, bool val)
{
	U64 bitPos = GetComponentBitPosition<Type>();
	if (val) { mask.SetBit(bitPos); }
	else { mask.ClearBit(bitPos); }
}

template <class Type>
inline void Scene::AddComponentBit()
{
	U64* index = componentBitPosition.Request(NameOf<Type>);
	*index = componentPools.Size();
}

template <class... Types>
inline Bitset Scene::GetMask()
{
	Bitset mask;
	(SetComponentBit<Types>(mask, 1), ...);
	return mask;
}

template<class Type>
inline void Scene::LoadComponents()
{
	if constexpr (IsComponent<Type>)
	{
		View<Type> view = GetView<Type>();

		view.ForEach([&](U32 id, Type& t)
		{
			t.Load(this, id);
		});
	}
}

template <class... Types>
inline View<Types...> Scene::GetView()
{
	Array<SparseSetBase*, sizeof...(Types)> arr = { GetComponentPoolPtr<Types>()... };

	return { arr };
}

template<class Type, typename... Args>
inline Type* Entity::AddComponent(Args&&... args) noexcept
{
	return scene->AddComponent<Type>(id, Forward<Args>(args)...);
}

template<class Type>
inline void Entity::RemoveComponent()
{
	scene->DestroyComponent<Type>(id);
}

template<class Type>
inline Type* Entity::GetComponent()
{
	return scene->GetComponent<Type>(id);
}

template<class Type>
SparseSet<Type>::SparseSet()
{
	dense.Reserve(256);
	denseToEntity.Reserve(256);
}

template<class Type>
template<typename... Args>
inline Type* SparseSet<Type>::Emplace(U32 id, Args&&... args)
{
	U32 index = GetDenseIndex(id);
	if (index != Invalid)
	{
		dense.EmplaceAt(index, Forward<Args>(args)...);
		denseToEntity[index] = id;

		return dense.Data() + index;
	}

	SetDenseIndex(id, (U32)dense.Size());

	dense.Emplace(Forward<Args>(args)...);
	denseToEntity.Push(id);

	return &dense.Back();
}

template<class Type>
inline Type* SparseSet<Type>::Add(U32 id, Type&& obj)
{
	U32 index = GetDenseIndex(id);
	if (index != Invalid)
	{
		dense[index] = Move(obj);
		denseToEntity[index] = id;

		return dense.Data() + index;
	}

	SetDenseIndex(id, (U32)dense.Size());

	dense.Push(obj);
	denseToEntity.Push(id);

	return &dense.Back();
}

template<class Type>
inline Type* SparseSet<Type>::Get(U32 id)
{
	U32 index = GetDenseIndex(id);

	if (index == Invalid) { return nullptr; }
	else { return dense.Data() + index; }
}

template<class Type>
inline Type& SparseSet<Type>::GetRef(U32 id)
{
	U32 index = GetDenseIndex(id);
	if (index == Invalid) { BreakPoint; }
	return dense[index];
}

template<class Type>
inline void SparseSet<Type>::Remove(U32 id)
{
	U32 deletedIndex = GetDenseIndex(id);

	if (dense.Empty() || deletedIndex == Invalid) { return; }

	SetDenseIndex(denseToEntity.Back(), deletedIndex);
	SetDenseIndex(id, Invalid);

	Swap(dense.Back(), dense[deletedIndex]);
	Swap(denseToEntity.Back(), denseToEntity[deletedIndex]);

	dense.Pop();
	denseToEntity.Pop();
}

template<class Type>
inline void SparseSet<Type>::SetDenseIndex(U32 id, U32 index)
{
	U32 page = id / PageSize;
	U32 sparseIndex = id % PageSize;

	if (page >= pages.Size())
	{
		pages.Resize(page + 1);
		for (U32& i : pages[page]) { i = Invalid; }
	}

	Sparse& sparse = pages[page];

	sparse[sparseIndex] = index;
}

template<class Type>
inline U32 SparseSet<Type>::GetDenseIndex(U32 id)
{
	U32 page = id / PageSize;
	U32 sparseIndex = id % PageSize;

	if (page < pages.Size())
	{
		Sparse& sparse = pages[page];
		return sparse[sparseIndex];
	}

	return Invalid;
}

template<class Type>
inline U32 SparseSet<Type>::Size()
{
	return (U32)dense.Size();
}

template<class Type>
inline Vector<U32> SparseSet<Type>::IndexList()
{
	return denseToEntity;
}

template<class Type>
inline bool SparseSet<Type>::ContainsIndex(U32 id)
{
	return GetDenseIndex(id) != Invalid;
}

template<class Type>
inline void SparseSet<Type>::Clear()
{
	dense.Clear();
	pages.Clear();
	denseToEntity.Clear();
}

template<class Type>
inline void SparseSet<Type>::Load(Scene* scene)
{
	scene->LoadComponents<Type>();
}

template<class Type>
inline bool SparseSet<Type>::IsEmpty() const
{
	return dense.Empty();
}

template<class Type>
inline const Vector<Type>& SparseSet<Type>::Data() const
{
	return dense;
}

template<typename... Types>
inline View<Types...>::View(Array<SparseSetBase*, sizeof...(Types)> pools) : viewPools{ pools }
{
	smallest = viewPools[0];

	for (SparseSetBase* base : viewPools)
	{
		if (base->Size() < smallest->Size()) { smallest = base; }
	}
}

template<typename... Types>
inline void View<Types...>::ForEach(ForEachFunc func)
{
	ForEachImpl(func);
}

template<typename... Types>
inline void View<Types...>::ForEach(ForEachFuncWithID func)
{
	ForEachImpl(func);
}

template<typename... Types>
inline Vector<typename View<Types...>::Pack> View<Types...>::GetPacked()
{
	auto inds = MakeIndexSequence<sizeof...(Types)>{};
	Vector<Pack> result;

	for (U32 id : smallest->IndexList())
		if (AllContain(id))
			result.Push({ id, MakeComponentTuple(id, inds) });
	return result;
}

template<typename... Types>
inline bool View<Types...>::AllContain(U32 id)
{
	for (SparseSetBase* pool : viewPools)
	{
		if (!pool->ContainsIndex(id)) { return false; }
	}

	return true;
}

template<typename... Types>
template <U64 Index>
inline auto View<Types...>::GetPoolAt()
{
	using componentType = typename types::template Get<Index>;
	return static_cast<SparseSet<componentType>*>(viewPools[Index]);
}

template<typename... Types>
template <U64... Indices>
inline auto View<Types...>::MakeComponentTuple(U32 id, IndexSequence<Indices...>)
{
	return std::make_tuple((std::ref(GetPoolAt<Indices>()->GetRef(id)))...);
}

template<typename... Types>
template <typename Func>
inline void View<Types...>::ForEachImpl(Func func)
{
	auto inds = MakeIndexSequence<sizeof...(Types)>{};

	for (U32 id : smallest->IndexList())
	{
		if (AllContain(id))
		{
			if constexpr (IsInvocable<Func, U32, Types&...>)
			{
				std::apply(func, std::tuple_cat(std::make_tuple(id), MakeComponentTuple(id, inds)));
			}
			else if constexpr (IsInvocable<Func, Types&...>)
			{
				std::apply(func, MakeComponentTuple(id, inds));
			}
		}
	}
}