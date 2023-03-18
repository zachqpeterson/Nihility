#include "Engine.hpp"

#include "Memory\Memory.hpp"
#include "Platform\Platform.hpp"
#include "Platform\Input.hpp"
#include "Platform\Jobs.hpp"
#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Containers\String.hpp"
#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"

#include <stdio.h>

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

void work(void* data)
{
	Logger::Info((C8*)data);
}

void Engine::Initialize(const C8* applicationName, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;
	suspended = false;

	ASSERT(Time::Initialize());
	ASSERT(Memory::Initialize());
	ASSERT(Logger::Initialize());

	U8 c00 = U8_MAX;
	U8 c01 = U8_MIN;
	I8 c02 = I8_MAX;
	I8 c03 = I8_MIN;
	U16 c10 = U16_MAX;
	U16 c11 = U16_MIN;
	I16 c12 = I16_MAX;
	I16 c13 = I16_MIN;
	U32 c20 = U32_MAX;
	U32 c21 = U32_MIN;
	I32 c22 = I32_MAX;
	I32 c23 = I32_MIN;
	UL32 c30 = UL32_MAX;
	UL32 c31 = UL32_MIN;
	L32 c32 = L32_MAX;
	L32 c33 = L32_MIN;
	U64 c40 = U64_MAX;
	U64 c41 = U64_MIN;
	I64 c42 = I64_MAX;
	I64 c43 = I64_MIN;
	F32 c50 = 0.0f;
	F32 c51 = 123.123f;
	F64 c52 = 0.0;
	F64 c53 = 123.123;

	String s0("{}",  c00);
	String s1("{}",  c01);
	String s2("{}",  c02);
	String s3("{}",  c03);
	String s4("{h}", c00);
	String s5("{h}", c01);
	String s6("{h}", c02);
	String s7("{h}", c03);

	String s10("{}",  c10);
	String s11("{}",  c11);
	String s12("{}",  c12);
	String s13("{}",  c13);
	String s14("{h}", c10);
	String s15("{h}", c11);
	String s16("{h}", c12);
	String s17("{h}", c13);

	String s20("{}",  c20);
	String s21("{}",  c21);
	String s22("{}",  c22);
	String s23("{}",  c23);
	String s24("{h}", c20);
	String s25("{h}", c21);
	String s26("{h}", c22);
	String s27("{h}", c23);

	String s30("{}",  c30);
	String s31("{}",  c31);
	String s32("{}",  c32);
	String s33("{}",  c33);
	String s34("{h}", c30);
	String s35("{h}", c31);
	String s36("{h}", c32);
	String s37("{h}", c33);

	String s40("{}",  c40);
	String s41("{}",  c41);
	String s42("{}",  c42);
	String s43("{}",  c43);
	String s44("{h}", c40);
	String s45("{h}", c41);
	String s46("{h}", c42);
	String s47("{h}", c43);
	
	String s50("{}",  c50);
	String s51("{}",  c51);
	String s52("{}",  c52);
	String s53("{}",  c53);
	String s54("{h}", c50);
	String s55("{h}", c51);
	String s56("{h}", c52);
	String s57("{h}", c53);

	//TODO: Only works with decimal count of 5 or 0
	String s58("{.}", c50);
	String s59("{.}", c51);
	String s510("{.}", c52);
	String s511("{.}", c53);
	String s512("{.3}", c50);
	String s513("{.3}", c51);
	String s514("{.3}", c52);
	String s515("{.3}", c53);
	String s516("{.0}", c50);
	String s517("{.0}", c51);
	String s518("{.0}", c52);
	String s519("{.0}", c53);

	String s60("asdasd");
	String s61(L"asdasd");
	String s62(u8"asdasd");
	String s63(u"asdasd");
	String s64(U"asdasd");

	String s70("{}{}{}{}", "Hello, ", 123, 425.534, 'a');

	Logger::Fatal("Hello, {}!", "World");
	Logger::Error("Hello, World!");
	Logger::Warn("Hello, World!");
	Logger::Info("Hello, World!");
	Logger::Trace("Hello, World!");
	Logger::Debug("Hello, World!");

	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));	//Thread
	ASSERT(Input::Initialize());					//Probably run on platform thread
	ASSERT(Resources::Initialize());
	//Audio
	//Renderer
	//UI
	//Physics
	//Particle
	ASSERT(GameInit());

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	GameShutdown();
	//Particle
	//Physics
	//UI
	//Renderer
	Resources::Shutdown();
	//Audio
	Input::Shutdown();
	Platform::Shutdown();
	Jobs::Shutdown();
	Settings::Shutdown();
	Logger::Shutdown();
	Memory::Shutdown();
	Time::Shutdown();
}

void Engine::UpdateLoop()
{
	while (running)
	{
		Time::Update();
		//Logger::Info("Framerate: {}", Time::FrameRate());

		if (!Platform::Update()) { break; } //TODO: Run on separate thread

		Input::Update(); //TODO: Run on separate thread
		//Physics::Update();
		GameUpdate();
		//UI::Update();
		//Animations::Update();
		//Renderer::Update(); //TODO: Checks for being minimised
		//Audio::Update();

		if (Settings::Focused)
		{
			F64 remainingFrameTime = Settings::TargetFrametime - Time::FrameUpTime();
			U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

			if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
		}
		else
		{
			F64 remainingFrameTime = Settings::TargetFrametimeSuspended - Time::FrameUpTime();
			U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

			if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
		}
	}
}

