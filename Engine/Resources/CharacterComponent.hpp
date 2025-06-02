#pragma once

#include "Component.hpp"

#include "Math/Physics.hpp"

struct b2ShapeId;
struct b2PlaneResult;

class NH_API Character
{
	static constexpr I32 planeCapacity = 8;

public:
	Vector2 capsuleCenter1 = { 0.0f, -0.5f };
	Vector2 capsuleCenter2 = { 0.0f, 0.5f };
	F32 capsuleRadius = 0.3f;

	Vector2 position = Vector2::Zero;
	Vector2 velocity = Vector2::Zero;
	F32 jumpForce = 15.0f;
	F32 minSpeed = 0.1f;
	F32 maxSpeed = 10.0f;
	F32 stopSpeed = 3.0f;
	F32 throttle = 0.0f;
	F32 acceleration = 20.0f;
	F32 friction = 8.0f;
	F32 gravity = 30.0f;
	F32 airSteering = 0.2f;
	F32 pogoHertz = 10.0f;
	F32 pogoDamping = 0.8f;
	F32 pogoVelocity = 0.0f;
	F32 pogoDampingRatio = 0.8f;
	I32 totalIterations = 0;
	I32 planeCount = 0;
	CollisionPlane planes[planeCapacity] = {};
	bool grounded = false;

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Character> AddTo(const EntityRef& entity);

private:
	static bool Update(U32 sceneId, Camera& camera, Vector<Entity>& entities);
	static bool Render(U32 sceneId, CommandBuffer commandBuffer);

	static bool PlaneResultFcn(b2ShapeId shapeId, const b2PlaneResult* planeResult, void* context);

	void ProcessInput();
	void Simulate();

	static bool initialized;

	COMPONENT(Character, 1);
	friend struct Scene;
	friend struct EntityRef;
};