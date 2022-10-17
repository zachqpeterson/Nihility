#pragma once

#include "World.hpp"

class Scene;
class Player;
class Inventory;
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

	static void PickupItem(U16 itemID, U16 amount);

private:
	static void LoadWorld();
	static void CreateWorld(UIElement* element, const Vector2Int& mousePos, void* data);

	static Scene* mainMenuScene;
	static Scene* worldScene;
	static World* world;

	static Player* player; //TODO: We will need to keep track of multiple players in multiplayer for rendering
	static Inventory* inventory; //But We only need to keep track of the inventory of your character

	static GameState gameState;

	static WorldSize smallWorldSize;
	static WorldSize mediumWorldSize;
	static WorldSize largeWorldSize;
};