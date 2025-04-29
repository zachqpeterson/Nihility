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

	ResourceRef<Model> model = Resources::LoadModel("models/Woman.gltf");
	ModelInstance instance = Resources::CreateModelInstance(model);
	Renderer::AddModelInstance(instance);

	MainLoop();
	Shutdown();

	return true;
}

void Engine::Shutdown()
{
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
	F64 timeAccumulation = 0.0;
	while (Platform::running)
	{
		Time::Update();
		Input::Update();
		Platform::Update();

		const Vector<ButtonEvent>& events = Input::GetInputEvents();

		for (const ButtonEvent& event : events)
		{
			if (event.code == ButtonCode::Escape && event.type == InputType::Press)
			{
				Platform::running = false;
			}
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
}