#pragma once

#include "ResourceDefines.hpp"

#include "Component.hpp"
#include "Scene.hpp"
#include "Math\Math.hpp"

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

struct Scene;

struct NH_API Entity
{
public:
	Entity(Entity&& other) noexcept : transform{ other.transform }, scene{ other.scene }, references{ Move(references) } {}
	Entity& operator=(Entity&& other) noexcept
	{
		transform = other.transform;
		scene = other.scene;
		references = Move(references);

		return *this;
	}

	template<ComponentType Type, typename... Args>
	Type* AddComponent(const Args&... args)
	{
		ComponentReference reference;
		Type* component = scene->AddComponent<Type>(reference, args...);
		references.Push(reference);

		return component;
	}

	Transform transform{};

private:
	Entity(Scene* scene) : scene{ scene } {}

	Scene* scene;
	Vector<ComponentReference> references{};

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	friend struct Scene;
};