#include "Defines.hpp"

#include "Engine.hpp"

#include "Rendering/Renderer.hpp"
#include "Resources/Resources.hpp"
#include "Platform/Input.hpp"
#include "Math/Random.hpp"
#include "Resources/SpriteComponent.hpp"
#include "Resources/RigidBodyComponent.hpp"
#include "Resources/ColliderComponent.hpp"
#include "Resources/TilemapComponent.hpp"
#include "Resources/CharacterComponent.hpp"
#include "Resources/Scene.hpp"

SceneRef scene;
ResourceRef<Texture> textureAtlas;
ResourceRef<Texture> groundTexture;

void ComponentsInit()
{
	Scene::InitializeFns += RigidBody::Initialize;
	Scene::InitializeFns += Character::Initialize;
	Scene::InitializeFns += Collider::Initialize;
	Scene::InitializeFns += Tilemap::Initialize;
	Scene::InitializeFns += Sprite::Initialize;

	Scene::ShutdownFns += Sprite::Shutdown;
	Scene::ShutdownFns += Tilemap::Shutdown;
	Scene::ShutdownFns += Collider::Shutdown;
	Scene::ShutdownFns += Character::Shutdown;
	Scene::ShutdownFns += RigidBody::Shutdown;
}

bool Initialize()
{
	scene = Scene::CreateScene(CameraType::Orthographic);

	textureAtlas = Resources::LoadTexture("textures/atlas.png");
	groundTexture = Resources::LoadTexture("textures/missing_texture.png");

	scene->LoadScene();

	EntityRef ground = scene->CreateEntity({ 0.0f, -30.0f }, { 100.0f, 3.0f });

	ComponentRef<RigidBody> rb = RigidBody::AddTo(ground, BodyType::Static);
	Collider::AddTo(ground, rb);
	Sprite::AddTo(ground, groundTexture);
	//ComponentRef<Tilemap> tm = Tilemap::AddTo(ground);
	//
	//for (I32 x = 0; x < 61; ++x)
	//{
	//	//for (I32 y = 0; y < 60; ++y)
	//	{
	//		tm->SetTile(groundTexture, { x, 31 });
	//	}
	//}

	EntityRef player = scene->CreateEntity();
	Sprite::AddTo(player, groundTexture);
	Character::AddTo(player);

	return true;
}

void Shutdown()
{
	scene.Destroy();
}

void Update()
{
	if (Input::ButtonDown(ButtonCode::E))
	{
		Vector2 position;
		position.x = F32(Random::RandomUniform() * 100.0f - 50.0f);
		position.y = F32(Random::RandomUniform() * 60.0f - 30.0f);

		EntityRef id = scene->CreateEntity(position);

		F32 x = Random::RandomRange(0, 2) / 2.0f;
		F32 y = Random::RandomRange(0, 2) / 2.0f;

		Sprite::AddTo(id, textureAtlas, Vector4::One, Vector2{ x, y }, Vector2{ 0.5f, 0.5f });
		ComponentRef<RigidBody> rb = RigidBody::AddTo(id, BodyType::Dynamic);
		Collider::AddTo(id, rb);
	}
}

int main()
{
	GameInfo game{
		.name = "Nihilty Demo",
		.version = MakeVersionNumber(0, 1, 0),
		.componentsInit = ComponentsInit,
		.initialize = Initialize,
		.shutdown = Shutdown,
		.update = Update,
	};

	Engine::Initialize(game);

	return 0;
}