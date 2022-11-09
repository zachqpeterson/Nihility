#pragma once

#include "World.hpp"

#include <Containers/HashTable.hpp>
#include <Containers/List.hpp>

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
struct Recipe;

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

	static void PickupItem(U16 itemID, U16 amount);

	static Transform2D* GetTarget(Transform2D* position, F32 range);
	static void Attack(const Damage& damage, const Vector4& area);

	static Vector2Int MouseToWorld();
	static bool MouseToWorldInRange(Vector2Int& pos, U8 range);

	static void OnClickSelect(UIElement* e, const Vector2Int& pos, void* data);
	static void OnClickCraft(UIElement* e, const Vector2Int& pos, void* data);

private:
	static void CreateWorld(UIElement* element, const Vector2Int& mousePos, void* data);
	static void LoadWorld();

	static void HandleInput();
	static void HandleEntities();
	static void UpdateDayCycle();

	static void CreateInventory();
	static void CreateCraftingMenu();
	static void UpdateCraftingMenu();

	static Scene* mainMenuScene;
	static Scene* worldScene;
	static World* world;

	static F32 timeScale;
	static F32 currentTime;
	static bool night;
	static Vector3 globalColor;

	static Player* player; //Keep track of your character
	static Inventory* inventory; //We only need to keep track of the inventory of your character
	static Inventory* hotbar;
	static UIElement* hotbarHighlight;
	static UIElement* craftingMenu;
	static UIElement* craftResult;
	static UIElement* craftButton;
	static const Recipe* selectedRecipe;

	static Vector<Vector2> blankUVs;

	static U8 equippedSlot;

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