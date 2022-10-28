#pragma once

#include "World.hpp"

#include <Containers/HashTable.hpp>

class Scene;
class Player;
class Entity;
class Inventory;
struct Slot;
struct Damage;
struct Vector2;
struct Vector4;
struct UIElement;
struct Transform2D;

enum GameState
{
	GAME_STATE_NONE,
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
	static void HandleEntities();
	static void UpdateDayCycle();

	static void PickupItem(U16 itemID, U16 amount);

	static Transform2D* GetTarget(Transform2D* position);
	static void Attack(const Damage& damage, const Vector4& area);

private:
	static void LoadWorld();
	static void CreateWorld(UIElement* element, const Vector2Int& mousePos, void* data);

	static Scene* mainMenuScene;
	static Scene* worldScene;
	static World* world;

	static F32 timeScale;
	static F32 currentTime;
	static bool night;
	static Vector3 globalColor;

	static Player* player;
	static Inventory* inventory; //We only need to keep track of the inventory of your character

	static HashTable<U64, Entity*> entities;

	static U8 playerCount;

	static GameState gameState;
	static GameState nextState;

	static WorldSize testWorldSize;
	static WorldSize smallWorldSize;
	static WorldSize mediumWorldSize;
	static WorldSize largeWorldSize;

	TimeSlip() = delete;
};