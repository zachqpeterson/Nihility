#include "TimeSlip.hpp"

#include <Engine.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Scene.hpp>
#include <Physics/Physics.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>

Scene* TimeSlip::mainMenuScene;
Scene* TimeSlip::worldScene;
World* TimeSlip::world;

bool TimeSlip::Initialize()
{
	mainMenuScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	mainMenuScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);

	//UI

	worldScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	worldScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	RendererFrontend::UseScene(worldScene);

	CreateWorld(WS_TEST);

	return true;
}

void TimeSlip::Shutdown()
{

}

bool TimeSlip::Update()
{
	Vector3 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), (F32)(Input::ButtonDown(S) - Input::ButtonDown(W)), 0.0f };
	move *= (F32)(Time::DeltaTime() * 10.0f);

	worldScene->GetCamera()->Translate(move);

	return true;
}

void TimeSlip::LoadWorld()
{

}

void TimeSlip::CreateWorld(WorldSize size)
{
	world = new World(0, size);
}