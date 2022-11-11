#include "Enemy.hpp"

#include <Math/Math.hpp>
#include <Resources/Resources.hpp>
#include <Memory/Memory.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Physics/Physics.hpp>
#include <Resources/Animations.hpp>

struct Vertex
{
	Vector3 position;
	Vector2 uv;
};

Entity::Entity(const EntityConfig& config, bool player) : maxHealth{ config.maxHealth }, health{ config.maxHealth }, armor{ config.armor },
damageReduction{ config.damageReduction }, knockbackReduction{ config.knockbackReduction }, healthRegen{ config.healthRegen }, 
facing{ true }, ignore{ config.ignore }, player{ player }, despawnRange{ config.despawnRange }
{
	static U64 id = 0;

	String name{ "Entity{}", ++id };

	MeshConfig meshConfig{};
	meshConfig.MaterialName = "Entity.mat";
	meshConfig.vertices = Memory::Allocate(sizeof(Vertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(Vertex);
	meshConfig.vertexCount = 4;
	meshConfig.indices.Resize(6);
	meshConfig.name = name;

	Vertex* vertices = (Vertex*)meshConfig.vertices;

	vertices[0] = Vertex{ {-0.5f, -1.0f, 0.0f}, {0.0f, 0.333f} };
	vertices[1] = Vertex{ { 0.5f, -1.0f, 0.0f}, {0.1f, 0.333f} };
	vertices[2] = Vertex{ { 0.5f,  1.0f, 0.0f}, {0.1f, 0.0f} };
	vertices[3] = Vertex{ {-0.5f,  1.0f, 0.0f}, {0.0f, 0.0f} };

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
	physicsConfig.transform = new Transform2D(config.position);
	physicsConfig.trigger = false;
	physicsConfig.freezeRotation = true;
	physicsConfig.layerMask = 1;
	physicsConfig.type = BOX_COLLIDER;
	physicsConfig.box = { {-0.5f, 0.5f}, {-1.0f, 1.0f} };

	GameObject2DConfig goConfig{};
	goConfig.model = Resources::CreateModel(name, Vector<Mesh*>{1, Resources::CreateMesh(meshConfig)});
	goConfig.name = name;
	goConfig.physics = Physics::Create2DPhysicsObject(physicsConfig);
	goConfig.transform = physicsConfig.transform;

	gameObject = Resources::CreateGameObject2D(goConfig);
	animation = Animations::AddAnimation(gameObject->model->meshes[0], 0, 12, 10, 3, 10);

	RendererFrontend::DrawGameObject(gameObject);
}

Entity::~Entity()
{
	Destroy();
}

void Entity::Destroy()
{
	Animations::RemoveAnimation(animation);
	RendererFrontend::UndrawGameObject(gameObject);
	Resources::DestroyModel(gameObject->model);
	Physics::DestroyPhysicsObjects2D(gameObject->physics);
	Resources::DestroyGameObject2D(gameObject);
	gameObject = nullptr;
}

bool Entity::TakeDamage(const Damage& damage)
{
	if (!ignore)
	{
		health -= (damage.damage - Math::Max(armor - damage.armorPierce, 0.0f)) * (1.0f + damage.critMulti * (Math::RandomF() < damage.critChance));

		//TODO: Apply knockback

		DamageResponse();

		return health <= 0.0f;
	}

	return false;
}