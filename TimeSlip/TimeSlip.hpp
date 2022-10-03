#pragma once

#include "World.hpp"

class Scene;

class TimeSlip
{
public:
	static bool Initialize();
	static void Shutdown();
	static bool Update();

private:
	static void LoadWorld();
	static void CreateWorld(WorldSize size);

	static Scene* mainMenuScene;
	static Scene* worldScene;
	static World* world;
};