#pragma once

#include "Component.hpp"

#include "Math/Physics.hpp"
#include "Core/Events.hpp"

class NH_API Projectile
{
public:
	AABB collider;

	Vector2 position;
	Vector2 velocity;
	F32 acceleration = 0.0f;
	F32 gravity = 0.0f;
	F32 timer = 0.0f;
	bool expire = false;
	bool hit = false;
	bool hitVertical = false;
	Event<const EntityRef&, bool> OnHit;
	Event<const EntityRef&> OnExpire;
	Event<const EntityRef&> OnUpdate;

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Projectile> AddTo(const EntityRef& entity, const Vector2& velocity, F32 duration = 0.0f, F32 acceleration = 0.0f, F32 gravity = 0.0f);
	static void RemoveFrom(const EntityRef& entity);

private:
	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	void Simulate();

	static bool initialized;

	COMPONENT(Projectile);
	friend struct EntityRef;
};