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
#include "Multithreading/Jobs.hpp"
#include "Rendering/Renderer.hpp"

#include "Core/Formatting.hpp"

bool Engine::Initialize()
{
	Logger::Initialize();
	Memory::Initialize();
	Settings::Initialize();
	Platform::Initialize("Nihility Demo");
	Input::Initialize();
	Renderer::Initialize();
	Resources::Initialize();
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

	SpriteInstance* player = scene.AddSprite(playerTexture, { Vector2::Zero, Vector2::One * 3.0f, Quaternion2::Identity });
	F32 movespeed = 5.0f;
	F32 sprintModifier = 2.0f;

	//Initial transfer
	Renderer::SubmitTransfer();

	F64 timeAccumulation = 0.0;
	while (Platform::running)
	{
		Time::Update();
		Input::Update();
		Platform::Update();

		if (Input::OnButtonDown(ButtonCode::Escape)) { Platform::running = false; }

		if (Input::ButtonDown(ButtonCode::E))
		{
			Transform t{};
			t.position.x = Random::RandomUniform() * 100.0f - 50.0f;
			t.position.y = Random::RandomUniform() * 60.0f - 30.0f;
			t.scale = Vector2::One * 5.0f;

			F32 x = Random::RandomRange(0, 2) / 2.0f;
			F32 y = Random::RandomRange(0, 2) / 2.0f;

			scene.AddSprite(textureAtlas, t, Vector4::One, { x, y }, { 0.5f, 0.5f });
		}

		F32 sprint = 1.0f;

		if (Input::ButtonDown(ButtonCode::Shift))
		{
			sprint = sprintModifier;
		}

		if (Input::ButtonDown(ButtonCode::W))
		{
			player->transform.position += Vector2::Up * movespeed * sprint * Time::DeltaTime();
		}

		if (Input::ButtonDown(ButtonCode::S))
		{
			player->transform.position += Vector2::Down * movespeed * sprint * Time::DeltaTime();
		}

		if (Input::ButtonDown(ButtonCode::A))
		{
			player->transform.position += Vector2::Left * movespeed * sprint * Time::DeltaTime();
		}

		if (Input::ButtonDown(ButtonCode::D))
		{
			player->transform.position += Vector2::Right * movespeed * sprint * Time::DeltaTime();
		}

		//game update

		//physics update

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