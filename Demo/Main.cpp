#include "Defines.hpp"

#include "Engine.hpp"

#include "Rendering/Renderer.hpp"
#include "Resources/Resources.hpp"
#include "Platform/Input.hpp"
#include "Math/Random.hpp"
#include "Resources/SpriteComponent.hpp"

Scene scene;
ResourceRef<Texture> textureAtlas;
ResourceRef<Texture> groundTexture;

bool Initialize()
{
	scene.Create(CameraType::Orthographic);

	textureAtlas = Resources::LoadTexture("textures/atlas.png");
	groundTexture = Resources::LoadTexture("textures/missing_texture.png");

	Renderer::SetScene(&scene);

	EntityRef ground = scene.CreateEntity({ 0.0f, -30.0f });

	ground.AddComponent<SpriteComponent>(groundTexture, Vector2{ 100.0f, 3.0f });

	scene.AddRigidBody(ground, BodyType::Static);
	scene.AddCollider(ground, { 100.0f, 3.0f });

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

		EntityRef id = scene.CreateEntity(position);

		F32 x = Random::RandomRange(0, 2) / 2.0f;
		F32 y = Random::RandomRange(0, 2) / 2.0f;

		id.AddComponent<SpriteComponent>(textureAtlas, Vector2::One, Vector4::One, Vector2{ x, y }, Vector2{ 0.5f, 0.5f });

		scene.AddRigidBody(id, BodyType::Dynamic);
		scene.AddCollider(id);
	}
}

int main()
{
	GameInfo game{
		.name = "Nihilty Demo",
		.version = MakeVersionNumber(0, 1, 0),
		.initialize = Initialize,
		.shutdown = Shutdown,
		.update = Update,
	};

	Engine::Initialize(game);

	return 0;
}