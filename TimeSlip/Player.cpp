#include "Player.hpp"

#include <Math/Math.hpp>
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

Player::Player(const Vector2& position) : gameObject{ nullptr }
{
	MeshConfig meshConfig{};
	meshConfig.MaterialName = "Player.mat";
	meshConfig.vertices = Memory::Allocate(sizeof(Vertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(Vertex);
	meshConfig.vertexCount = 4;
	meshConfig.indices.Resize(6);
	meshConfig.name = "PlayerMesh";

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
	config.model = Resources::CreateModel("PlayerModel", Vector<Mesh*>{1, Resources::CreateMesh(meshConfig)});
	config.name = "Player"; //TODO: player numbering
	config.physics = Physics::Create2DPhysicsObject(physicsConfig);
	config.transform = physicsConfig.transform;

	gameObject = Resources::CreateGameObject2D(config);

	RendererFrontend::DrawGameObject(gameObject);
}

Player::~Player()
{
	Destroy();
}

void Player::Destroy()
{
	Resources::DestroyGameObject2D(gameObject);
	gameObject = nullptr;
}

void* Player::operator new(U64 size) { return Memory::Allocate(sizeof(Player), MEMORY_TAG_GAME); }
void Player::operator delete(void* ptr) { Memory::Free(ptr, sizeof(Player), MEMORY_TAG_GAME); }

void Player::Update()
{
	Vector2 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), 0.0f };
	move *= (F32)(Time::DeltaTime() * 10.0f);

	gameObject->physics->SetGravityScale(0.5f + 0.5f * !Input::ButtonDown(SPACE));

	if (Input::ButtonDown(SPACE) && gameObject->physics->Grounded())
	{
		gameObject->physics->ApplyForce({ 0.0f, -1.0f });
	}

	gameObject->physics->Translate(move);
}

void Player::SetPosition(const Vector2& position)
{
	gameObject->physics->Translate(gameObject->transform->Position() - position);
}