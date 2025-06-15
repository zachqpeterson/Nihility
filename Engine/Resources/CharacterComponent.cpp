#include "CharacterComponent.hpp"

#include "Scene.hpp"

#include "Platform/Input.hpp"
#include "Core/Time.hpp"

#include "box2d/box2d.h"

Vector<Vector<Character>> Character::components;
bool Character::initialized = false;

bool Character::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Character::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Character> Character::AddTo(const EntityRef& entity)
{
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<Character>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max Character Count Reached!");
		return {};
	}

	U32 instanceId = (U32)instances.Size();

	Character character{};
	character.entityIndex = entity.EntityId();

	instances.Push(character);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}

bool Character::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<Character>& instances = components[sceneId];

	for (Character& character : instances)
	{
		Entity& entity = entities[character.entityIndex];

		character.ProcessInput();
		character.Simulate();

		entity.position = character.position;

		camera.Follow(entity.position);
	}

	return false;
}

bool Character::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	return false;
}

void Character::ProcessInput()
{
	throttle = 0.0f;

	if(Input::ButtonDown(ButtonCode::A)) { throttle -= 1.0f; }
	if(Input::ButtonDown(ButtonCode::D)) { throttle += 1.0f; }

	if (Input::OnButtonDown(ButtonCode::Space))
	{
		if (grounded)
		{
			velocity.y = jumpForce;
			grounded = false;
		}
	}
}

void Character::Simulate()
{
	F32 dt = (F32)Time::DeltaTime();

	F32 speed = velocity.Magnitude();
	if (speed < minSpeed) { velocity = Vector2::Zero; }
	else if (grounded)
	{
		F32 control = speed < stopSpeed ? stopSpeed : speed;

		F32 drop = control * friction * dt;
		F32 newSpeed = Math::Max(0.0f, speed - drop);
		velocity *= newSpeed / speed;
	}

	Vector2 desiredVelocity = { maxSpeed * throttle, 0.0f };
	F32 desiredSpeed;
	Vector2 desiredDirection = desiredVelocity.Normalized(desiredSpeed);

	desiredSpeed = Math::Min(desiredSpeed, maxSpeed);

	if (grounded) { velocity.y = 0.0f; }

	F32 currentSpeed = velocity.Dot(desiredDirection);
	F32 addSpeed = desiredSpeed - currentSpeed;

	if (addSpeed > 0.0f)
	{
		F32 steer = grounded ? 1.0f : airSteering;
		F32 accelSpeed = steer * acceleration * maxSpeed * dt;

		accelSpeed = Math::Min(accelSpeed, addSpeed);

		velocity += desiredDirection * accelSpeed;
	}

	velocity.y -= gravity * dt;

	F32 pogoRestLength = 3.0f * capsuleRadius;
	F32 rayLength = pogoRestLength + capsuleRadius;
	Vector2 origin = capsuleCenter1 + position;
	F32 circleRadius = 0.5f * capsuleRadius;
	Vector2 segmentOffset = { 0.75f * capsuleRadius, 0.0f };
	Vector2 segmentPoints[2] = { origin - segmentOffset, origin + segmentOffset };

	ShapeProxy proxy = TypePun<ShapeProxy>(b2MakeProxy((b2Vec2*)segmentPoints, 2, 0.0f));
	Vector2 translation = { 0.0f, -rayLength };

	QueryFilter pogoFilter = { MoverBit, StaticBit | DynamicBit };
	CastResult castResult = Physics::ShapeCast(proxy, translation, pogoFilter);

	if (grounded == false) { grounded = castResult.hit && velocity.y <= 0.01f; }
	else { grounded = castResult.hit; }

	if (castResult.hit == false) { pogoVelocity = 0.0f; }
	else
	{
		F32 pogoCurrentLength = castResult.fraction * rayLength;

		F32 zeta = pogoDampingRatio;
		F32 hertz = pogoHertz;
		F32 omega = 2.0f * (F32)Pi * hertz;
		F32 omegaH = omega * dt;

		pogoVelocity = (pogoVelocity - omega * omegaH * (pogoCurrentLength - pogoRestLength)) /
			(1.0f + 2.0f * zeta * omegaH + omegaH * omegaH);

		b2Body_ApplyForce(TypePun<b2BodyId>(castResult.bodyId), { 0.0f, -50.0f }, TypePun<b2Vec2>(castResult.point), true);
	}

	Vector2 target = position + velocity * dt + Vector2{ 0.0f, pogoVelocity * dt };

	QueryFilter collideFilter = { MoverBit, StaticBit | DynamicBit | MoverBit };
	QueryFilter castFilter = { MoverBit, StaticBit | DynamicBit };

	totalIterations = 0;
	F32 tolerance = 0.01f;

	for (I32 iteration = 0; iteration < 5; ++iteration)
	{
		planeCount = 0;

		Capsule mover;
		mover.center1 = capsuleCenter1 + position;
		mover.center2 = capsuleCenter2 + position;
		mover.radius = capsuleRadius;

		Physics::CollideMover(mover, collideFilter, PlaneResultFcn, this);
		PlaneSolverResult result = Physics::SolvePlanes(target, planes, planeCount);

		totalIterations += result.iterationCount;

		Vector2 moverTranslation = result.position - position;

		F32 fraction = Physics::CastMover(mover, moverTranslation, castFilter);

		Vector2 delta = moverTranslation * fraction;
		position += delta;

		if (delta.SqrMagnitude() < tolerance * tolerance) { break; }
	}

	velocity = Physics::ClipVector(velocity, planes, planeCount);
}

struct ShapeUserData
{
	F32 maxPush;
	bool clipVelocity;
};

bool Character::PlaneResultFcn(b2ShapeId shapeId, const b2PlaneResult* planeResult, void* context)
{
	Character* self = (Character*)context;
	F32 maxPush = F32_MAX;
	bool clipVelocity = true;
	ShapeUserData* userData = (ShapeUserData*)b2Shape_GetUserData(shapeId);
	if (userData != nullptr)
	{
		maxPush = userData->maxPush;
		clipVelocity = userData->clipVelocity;
	}

	if (self->planeCount < planeCapacity)
	{
		self->planes[self->planeCount] = { TypePun<Plane>(planeResult->plane), maxPush, 0.0f, clipVelocity };
		self->planeCount += 1;
	}

	return true;
}