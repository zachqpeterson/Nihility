#pragma once

#include "World.hpp"

class Scene;
class Player;
struct Vector2;
struct UIElement;

enum GameState
{
	GAME_STATE_MENU,
	GAME_STATE_GAME,
	GAME_STATE_PAUSE,

	GAME_STATE_COUNT
};

class TimeSlip
{
public:
	static bool Initialize();
	static void Shutdown();
	static bool Update();

private:
	static void LoadWorld();
	static void CreateWorld(UIElement* element, ...);

	static Scene* mainMenuScene;
	static Scene* worldScene;
	static World* world;
	static Player* player;

	static GameState gameState;

	static WorldSize smallWorldSize;
	static WorldSize mediumWorldSize;
	static WorldSize largeWorldSize;
};