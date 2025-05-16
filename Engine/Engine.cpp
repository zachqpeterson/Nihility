#include "Engine.hpp"

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Platform/Platform.hpp"
#include "Platform/Memory.hpp"
#include "Platform/Input.hpp"
#include "Resources/Settings.hpp"
#include "Resources/Resources.hpp"
#include "Containers/String.hpp"
#include "Core/Time.hpp"
#include "Core/File.hpp"
#include "Core/Logger.hpp"
#include "Core/Events.hpp"
#include "Math/Math.hpp"
#include "Math/Random.hpp"
#include "Math/Physics.hpp"
#include "Multithreading/Jobs.hpp"
#include "Rendering/Renderer.hpp"

#include "Core/Formatting.hpp"

#include "tracy/Tracy.hpp"

bool Engine::Initialize()
{
	Logger::Initialize();
	Memory::Initialize();
	Settings::Initialize();
	Platform::Initialize("Nihility Demo");
	Input::Initialize();
	Renderer::Initialize();
	Resources::Initialize();
	Physics::Initialize();
	Time::Initialize();

	//TODO: Initialize Game

	MainLoop();
	Shutdown();

	return true;
}

void Engine::Shutdown()
{
	//TODO: Shutdown Game

	Time::Shutdown();
	Physics::Shutdown();
	Resources::Shutdown();
	Renderer::Shutdown();
	Input::Shutdown();
	Platform::Shutdown();
	Settings::Shutdown();
	Memory::Shutdown();
	Logger::Shutdown();
}

void Engine::MainLoop()
{
	Scene scene;
	scene.Create(CameraType::Orthographic);
	ResourceRef<Texture> textureAtlas = Resources::LoadTexture("textures/atlas.png");
	ResourceRef<Texture> playerTexture = Resources::LoadTexture("textures/missing_texture.png");

	Renderer::SetScene(&scene);

	EntityId ground = scene.CreateEntity({ 0.0f, -10.0f });

	scene.AddSprite(ground, playerTexture, { 100.0f, 3.0f });
	scene.AddRigidBody(ground, BodyType::Static);
	scene.AddCollider(ground, { 100.0f, 3.0f });

	//Initial transfer
	Renderer::SubmitTransfer();

	F64 timeAccumulation = 0.0;
	while (Platform::running)
	{
		FrameMark;
		Time::Update();
		Input::Update();
		Platform::Update();

		if (Input::OnButtonDown(ButtonCode::Escape)) { Platform::running = false; }

		if (Input::ButtonDown(ButtonCode::E))
		{
			Vector2 position;
			position.x = Random::RandomUniform() * 100.0f - 50.0f;
			position.y = Random::RandomUniform() * 60.0f - 30.0f;

			EntityId id = scene.CreateEntity(position);

			F32 x = Random::RandomRange(0, 2) / 2.0f;
			F32 y = Random::RandomRange(0, 2) / 2.0f;

			scene.AddSprite(id, textureAtlas, Vector2::One, Vector4::One, { x, y }, { 0.5f, 0.5f });
			scene.AddRigidBody(id, BodyType::Dynamic);
			scene.AddCollider(id);
		}

		//game update

		//physics update
		Physics::Update();

		Renderer::Update();

		F64 remainingFrameTime = Settings::targetFrametime - Time::FrameUpTime();
		I64 remainingUS = (I64)(remainingFrameTime * 1000000.0);

		while (remainingUS > 0)
		{
			Jobs::Yield();

			remainingFrameTime = Settings::targetFrametime - Time::FrameUpTime();
			remainingUS = (I64)(remainingFrameTime * 1000000.0);
		}
	}

	scene.Destroy();
}