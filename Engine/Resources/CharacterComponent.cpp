#include "CharacterComponent.hpp"

#include "World.hpp"

#include "Platform/Input.hpp"
#include "Core/Time.hpp"

Vector<Character> Character::components(1, {});
Freelist Character::freeComponents(1);
bool Character::initialized = false;

bool Character::Initialize()
{
	if (!initialized)
	{
		World::UpdateFns += Update;
		World::RenderFns += Render;

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
	U32 instanceId;
	Character& character = Create(instanceId);
	character.entityIndex = entity.EntityId();
	character.position = entity->position;
	character.collider = { { 1.0f, 1.5f }, { -1.0f, -1.5f } };

	return { entity.EntityId(), instanceId };
}

bool Character::Update(Camera& camera, Vector<Entity>& entities)
{
	for (Character& character : components)
	{
		if (character.entityIndex == U32_MAX) { continue; }
		Entity& entity = entities[character.entityIndex];

		character.ProcessInput();
		character.Simulate();

		entity.position = character.position;

		camera.Follow(entity.position);
	}

	return false;
}

bool Character::Render(CommandBuffer commandBuffer)
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
	F32 dt = Math::Min((F32)Time::DeltaTime(), 0.25f);
	
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

	Vector2 frameVelocity = velocity * dt;
	Vector2 target = position + frameVelocity;
	Collision collision;

	if (collision = Physics::CheckCollision(collider + position + Vector2{ frameVelocity.x, 0.0f }))
	{
		if (velocity.x > 0.0f) { position.x = collision.aabb.lowerBound.x - collider.upperBound.x; }
		if (velocity.x < 0.0f) { position.x = collision.aabb.upperBound.x + collider.upperBound.x; }
		
		velocity.x = 0.0f;
	}
	else
	{
		position.x = target.x;
	}

	if (collision = Physics::CheckCollision(collider + position + Vector2{ 0.0f, frameVelocity.y }))
	{
		if (velocity.y > 0.0f) { position.y = collision.aabb.lowerBound.y - collider.upperBound.y; velocity.y *= 0.95f; }
		if (velocity.y < 0.0f) { position.y = collision.aabb.upperBound.y + collider.upperBound.y; grounded = true; velocity.y = 0.0f; }
	}
	else
	{
		if (grounded) { jumpTimer = CoyoteTime; }
		grounded = false;
		position.y = target.y;
	}
}

void Character::AddForce(const Vector2& force)
{
	velocity += force;
}