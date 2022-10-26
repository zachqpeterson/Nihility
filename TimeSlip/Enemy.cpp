#include "Enemy.hpp"

#include "TimeSlip.hpp"

#include <Memory/Memory.hpp>
#include <Resources/Resources.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Physics/Physics.hpp>

struct Vertex
{
	Vector3 position;
	Vector2 uv;
};

Enemy::Enemy(const Vector2& position, EnemyAI aiType, bool aggressive) : aiType{ aiType }, target{ nullptr }, aggressive{ aggressive }, health{ 100.0f }
{
	static U64 id = 0;

	String name{ "Enemy{}", ++id };

	MeshConfig meshConfig{};
	meshConfig.MaterialName = "Player.mat";
	meshConfig.vertices = Memory::Allocate(sizeof(Vertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(Vertex);
	meshConfig.vertexCount = 4;
	meshConfig.indices.Resize(6);
	meshConfig.name = name;

	Vertex* vertices = (Vertex*)meshConfig.vertices;

	vertices[0] = Vertex{ {-0.5f, -1.0f, 1.0f}, {0.0f, 0.125f} };
	vertices[1] = Vertex{ { 0.5f, -1.0f, 1.0f}, {0.166666f, 0.125f} };
	vertices[2] = Vertex{ { 0.5f,  1.0f, 1.0f}, {0.166666f, 0.0f} };
	vertices[3] = Vertex{ {-0.5f,  1.0f, 1.0f}, {0.0f, 0.0f} };

	meshConfig.indices[0] = 0;
	meshConfig.indices[1] = 1;
	meshConfig.indices[2] = 2;
	meshConfig.indices[3] = 2;
	meshConfig.indices[4] = 3;
	meshConfig.indices[5] = 0;

	PhysicsObject2DConfig physicsConfig{};
	physicsConfig.density = 0.5;
	physicsConfig.gravityScale = 0.5;
	physicsConfig.kinematic = false;
	physicsConfig.restitution = 0.0;
	physicsConfig.friction = 0.2f;
	physicsConfig.transform = new Transform2D(position);
	physicsConfig.trigger = false;
	physicsConfig.freezeRotation = true;
	physicsConfig.layerMask = 1;
	physicsConfig.type = BOX_COLLIDER;
	physicsConfig.box = { {-0.5f, 0.5f}, {-1.0f, 1.0f} };

	GameObject2DConfig config{};
	config.model = Resources::CreateModel(name, Vector<Mesh*>{1, Resources::CreateMesh(meshConfig)});
	config.name = name;
	config.physics = Physics::Create2DPhysicsObject(physicsConfig);
	config.transform = physicsConfig.transform;

	gameObject = Resources::CreateGameObject2D(config);

	RendererFrontend::DrawGameObject(gameObject);
}

Enemy::~Enemy()
{
	Destroy();
}

void Enemy::Destroy()
{
	Resources::DestroyGameObject2D(gameObject);
	gameObject = nullptr;
	target = nullptr;
}

void* Enemy::operator new(U64 size) { return Memory::Allocate(sizeof(Enemy), MEMORY_TAG_GAME); }
void Enemy::operator delete(void* ptr) { Memory::Free(ptr, sizeof(Enemy), MEMORY_TAG_GAME); }

void Enemy::Update()
{
	if (aggressive && !target)
	{
		target = TimeSlip::GetTarget(gameObject->transform);
	}

	switch (aiType)
	{
	case ENEMY_AI_BASIC: { BasicAI(); }  break;
	case ENEMY_AI_FLYING: { FlyingAI(); } break;
	case ENEMY_AI_RANGED: { RangedAI(); } break;
	default: break;
	}

	lastPos = gameObject->transform->Position();
}

void Enemy::BasicAI()
{
	if (target)
	{
		Vector2 dir = target->Position() - gameObject->transform->Position();
		F32 dist = dir.Magnitude();

		if (dist > 10.0f)
		{
			startChase = false;
			target = nullptr;
		}
		else if (dist < 1.0f)
		{
			startChase = false;
			//TODO: Attack
		}
		else
		{
			Vector2 move{ (F32)Math::Sign(dir.x), 0.0f };
			move *= (F32)(Time::DeltaTime() * 5.0f);

			if (((gameObject->transform->Position() - lastPos).IsZero() && startChase) && gameObject->physics->Grounded())
			{
				gameObject->physics->ApplyForce({ 0.0f, -1.0f });
			}

			gameObject->physics->Translate(move);

			startChase = true;
		}
	}
}

void Enemy::FlyingAI()
{

}

void Enemy::RangedAI()
{

}

bool Enemy::TakeDamage(F32 amt)
{
	aggressive = true;

	health -= amt;
	return health <= 0.0f;
}