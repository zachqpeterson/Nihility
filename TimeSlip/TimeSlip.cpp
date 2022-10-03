#include "TimeSlip.hpp"

#include <Engine.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Scene.hpp>
#include <Physics/Physics.hpp>

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
	return true;
}

void TimeSlip::LoadWorld()
{

}

void TimeSlip::CreateWorld(WorldSize size)
{
	world = new World(0, size);
}