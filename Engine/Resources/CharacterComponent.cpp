#include "CharacterComponent.hpp"

#include "Scene.hpp"

#include "Platform/Input.hpp"
#include "Core/Time.hpp"

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
	character.position = entity->position;
	character.collider = { { 1.0f, 1.5f }, { -1.0f, -1.5f } };

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
	jumpTimer -= (F32)Time::DeltaTime();

	if(Input::ButtonDown(ButtonCode::A)) { throttle -= 1.0f; }
	if(Input::ButtonDown(ButtonCode::D)) { throttle += 1.0f; }

	if (Input::OnButtonDown(ButtonCode::Space))
	{
		if (grounded || jumpTimer > 0.0f)
		{
			velocity.y = jumpForce;
			jumpTimer = 0.0f;
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

	Vector2 target = position + velocity * dt;

	if (!Physics::CheckCollision(collider + position + Vector2{ velocity.x * dt, 0.0f }))
	{
		position.x = target.x;
	}
	else
	{
		velocity.x = 0.0f;
	}

	if (!Physics::CheckCollision(collider + position + Vector2{ 0.0f, velocity.y * dt }))
	{
		if (grounded) { jumpTimer = CoyoteTime; }
		grounded = false;
		position.y = target.y;
	}
	else
	{
		if (velocity.y < 0.0f) { grounded = true; }
		velocity.y = 0.0f;
	}
}