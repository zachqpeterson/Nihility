#pragma once

#include "World.hpp"

class Scene;
class Player;
struct Vector2;

class TimeSlip
{
public:
	static bool Initialize();
	static void Shutdown();
	static bool Update();

private:
	static void LoadWorld();
	static void CreateWorld(WorldSize size, Vector2& spawnPoint);

	static Scene* mainMenuScene;
	static Scene* worldScene;
	static World* world;
	static Player* player;
};