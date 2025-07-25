#pragma once

#include "Component.hpp"

#include "Math/Physics.hpp"

//TODO: Crouching
class NH_API Character
{
public:
	AABB collider;

	Vector2 position = Vector2::Zero;
	Vector2 velocity = Vector2::Zero;
	F32 jumpForce = 25.0f;
	F32 minSpeed = 0.1f;
	F32 maxSpeed = 10.0f;
	F32 sprintSpeed = 10.0f;
	F32 stopSpeed = 3.0f;
	F32 throttle = 0.0f;
	F32 acceleration = 20.0f;
	F32 friction = 8.0f;
	F32 gravity = 50.0f;
	F32 airSteering = 0.2f;
	bool grounded = false;
	bool sprinting = false;
	F32 jumpTimer = 0.0f;
	static constexpr F32 CoyoteTime = 0.15f;

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Character> AddTo(const EntityRef& entity, const Vector2& dimensions = { 0.6f, 1.2f });
	static void RemoveFrom(const EntityRef& entity);

	void AddForce(const Vector2& force);

private:
	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	void ProcessInput();
	void Simulate();

	static bool initialized;

	COMPONENT(Character);
	friend struct EntityRef;
};