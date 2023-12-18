#pragma once

#include "ResourceDefines.hpp"

#include "Mesh.hpp"
#include "Math\Math.hpp"
#include "Core\Logger.hpp"

#include <typeinfo>

//TODO: I think a good way to store components would be to have an array of type-id pairs. 
// Type is the type of components i.e. an id that points to an array of those components.
// Id is an index into the array of those components.
// This would make it trivial to add/remove components at runtime without polymorphism
// It also has the added benefit of having all components of a type in one array, making it more cache friendly
// To update components just iterate the array, it doesn't need to know about the parent object

struct NH_API Transform
{
public:
	const Matrix4& LocalMatrix()
	{
		if (dirty) { dirty = false; localMatrix.Set(position, rotation, scale); }

		return localMatrix;
	}

	const Matrix4& WorldMatrix()
	{
		if (dirty) { dirty = false; localMatrix.Set(position, rotation, scale); }

		if (parent)
		{
			worldMatrix = localMatrix * parent->WorldMatrix();
			return worldMatrix;
		}
		
		return localMatrix;
	}

	void Translate(const Vector3& translation)
	{
		position += translation;
		localMatrix.SetPosition(position);
	}

	void SetPosition(const Vector3& position)
	{
		this->position = position;
		localMatrix.SetPosition(position);
	}

	void SetRotation(const Quaternion3& rotation)
	{
		dirty = true;
		this->rotation = rotation;
	}

	void SetScale(const Vector3& scale)
	{
		dirty = true;
		this->scale = scale;
	}

private:
	bool dirty{ false };
	Transform* parent{ nullptr };
	Vector3 position{ Vector3Zero };
	Vector3 scale{ Vector3One };
	Quaternion3 rotation{ Quaternion3Identity };
	Matrix4 localMatrix{ Matrix4Identity };
	Matrix4 worldMatrix{ Matrix4Identity };
};

struct NH_API Component
{
	virtual void Update() = 0;
};

struct NH_API ModelComponent : public Component
{
	void Update() override
	{
		Logger::Debug("Bing Bong");
	}
};

struct ComponentPool
{
	virtual void Update() = 0;

	virtual Component* CreateComponent() = 0;
};

template <class Type> inline constexpr bool IsComponent = InheritsFrom<Type, Component>;
template <class Type> concept ComponentType = IsComponent<Type>;

template<ComponentType Type>
struct ComponentPoolInternal : public ComponentPool
{
	void Update() final
	{
		for (Type& component : components)
		{
			component.Update();
		}
	}

	Type* CreateComponent() final
	{
		components.Push(Type{});

		return &components.Back();
	}

	Vector<Type> components;
};

class NH_API Components
{
public:
	template<ComponentType Type>
	static void RegisterComponent()
	{
		U64 id = typeid(Type).hash_code();

		AddID(id);
		AddPool(new ComponentPoolInternal<Type>{});
	}

	template<ComponentType Type>
	static Type* CreateComponent()
	{
		U64 hash = typeid(Type).hash_code();

		return (Type*)GetNewComponent(hash);
	}

private:
	static void Update();

	static void AddID(U64 id);
	static void AddPool(ComponentPool* pool);
	static Component* GetNewComponent(U64 hash);

	static Hashmap<U64, U64> ids;
	static Vector<ComponentPool*> pools;

	STATIC_CLASS(Components);
	friend class Engine;
};

struct NH_API Entity
{
	Transform transform{};
	Model* model{};
};