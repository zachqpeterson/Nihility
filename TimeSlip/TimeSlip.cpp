#include "TimeSlip.hpp"

#include "Player.hpp"
#include "Inventory.hpp"
#include "Entity.hpp"
#include "Enemy.hpp"
#include "Tile.hpp"
#include "Items.hpp"

#include <Engine.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Scene.hpp>
#include <Resources/Resources.hpp>
#include <Resources/UI.hpp>
#include <Physics/Physics.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Math/Math.hpp>
#include <Platform/Platform.hpp>
#include <Audio/Audio.hpp>

#define MAX_ENTITIES 10

Scene* TimeSlip::mainMenuScene;
Scene* TimeSlip::worldScene;
World* TimeSlip::world;

F32 TimeSlip::timeScale;
F32 TimeSlip::currentTime;
bool TimeSlip::night;
Vector3 TimeSlip::globalColor;

UIElement* TimeSlip::titleImage;
UIElement* TimeSlip::playButton;
UIElement* TimeSlip::settingsButton;
UIElement* TimeSlip::exitButton;
UIElement* TimeSlip::smallWorldButton;
UIElement* TimeSlip::mediumWorldButton;
UIElement* TimeSlip::largeWorldButton;
UIElement* TimeSlip::backButton;

Player* TimeSlip::player;
Inventory* TimeSlip::inventory;
Inventory* TimeSlip::hotbar;
UIElement* TimeSlip::hotbarHighlight;
UIElement* TimeSlip::craftingMenu;
UIElement* TimeSlip::craftResult;
UIElement* TimeSlip::craftButton;
UIElement* TimeSlip::ingredientList;
UIElement* TimeSlip::recipeSelection;
const Recipe* TimeSlip::selectedRecipe;

Vector<Vector2> TimeSlip::blankUVs;

U8 TimeSlip::equippedSlot;

HashTable<U64, Entity*> TimeSlip::entities;

U8 TimeSlip::playerCount;

GameState TimeSlip::gameState;
GameState TimeSlip::nextState;

WorldSize TimeSlip::testWorldSize{ WS_TEST };
WorldSize TimeSlip::smallWorldSize{ WS_SMALL };
WorldSize TimeSlip::mediumWorldSize{ WS_MEDIUM };
WorldSize TimeSlip::largeWorldSize{ WS_LARGE };

bool TimeSlip::running;

void TimeSlip::OnClickSelect(UIElement* e, const Vector2Int& pos, void* data)
{
	const Recipe* recipe = (const Recipe*)data;
	selectedRecipe = recipe;

	UI::SetElementPosition(recipeSelection, Vector2{ -0.1f, e->area.y * 1.5f + 0.22f }); //TODO: Fix this
	UI::SetEnable(recipeSelection, true);
	UI::ChangeTexture(craftResult, nullptr, Inventory::GetUV(recipe->result));

	UpdateCraftingMenu();
}

void TimeSlip::OnClickCraft(UIElement* e, const Vector2Int& pos, void* data)
{
	const Recipe* recipe = (const Recipe*)data;

	for (U16 i = 0; i < recipe->ingredientCount; ++i)
	{
		if (recipe->ingredients[i].consumed) { inventory->RemoveItem(recipe->ingredients[i].id, recipe->ingredients[i].amount); }
	}

	PickupItem(recipe->result, recipe->amount);
}

void TimeSlip::MainMenu(UIElement* element, const Vector2Int& mousePos, void* data)
{
	UI::SetEnable(titleImage, true);
	UI::SetEnable(playButton, true);
	UI::SetEnable(settingsButton, true);
	UI::SetEnable(exitButton, true);
	UI::SetEnable(smallWorldButton, false);
	UI::SetEnable(mediumWorldButton, false);
	UI::SetEnable(largeWorldButton, false);
	UI::SetEnable(backButton, false);
}

void TimeSlip::PlayMenu(UIElement* element, const Vector2Int& mousePos, void* data)
{
	UI::SetEnable(titleImage, false);
	UI::SetEnable(playButton, false);
	UI::SetEnable(settingsButton, false);
	UI::SetEnable(exitButton, false);
	UI::SetEnable(smallWorldButton, true);
	UI::SetEnable(mediumWorldButton, true);
	UI::SetEnable(largeWorldButton, true);
	UI::SetEnable(backButton, true);
}

void TimeSlip::SettingsMenu(UIElement* element, const Vector2Int& mousePos, void* data)
{
	UI::SetEnable(titleImage, false);
	UI::SetEnable(playButton, false);
	UI::SetEnable(settingsButton, false);
	UI::SetEnable(exitButton, false);
	UI::SetEnable(backButton, true);
}

void TimeSlip::Exit(UIElement* element, const Vector2Int& mousePos, void* data)
{
	running = false;
}

bool TimeSlip::Initialize()
{
	static const F32 camWidth = 3.63636363636f;
	static const F32 camHeight = 2.04545454545f;
	blankUVs = { 4, Vector2::ZERO };

	running = true;
	gameState = GAME_STATE_MENU;
	nextState = GAME_STATE_NONE;
	mainMenuScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	mainMenuScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	RendererFrontend::UseScene(mainMenuScene);

	worldScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	worldScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	worldScene->GetCamera()->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);

	CreateMainMenu();

	entities(19);
	Resources::LoadAudio("Mine.wav");

	return true;
}

void TimeSlip::Shutdown()
{
	if (world) { world->Destroy(); }

	for (Entity* entity : entities)
	{
		entity->Destroy();
	}

	entities.Destroy();

	Inventory::Shutdown();
}

bool TimeSlip::Update()
{
	switch (gameState)
	{
	case GAME_STATE_MENU: break;
	case GAME_STATE_GAME: {
		if (Input::OnButtonDown(I)) { inventory->ToggleShow(); }
		if (Input::OnButtonDown(C)) { UI::SetEnable(craftingMenu, !craftingMenu->selfEnabled); }
		Inventory::Update();
		HandleInput();
		world->Update();
		HandleEntities();
		UpdateDayCycle();
		worldScene->GetCamera()->Update();
	}	break;
	case GAME_STATE_PAUSE: break;
		//TODO: Pause Physics
	default: break;
	}

	if (nextState) { gameState = nextState; nextState = GAME_STATE_NONE; }

	return running;
}

void TimeSlip::HandleInput()
{
	constexpr U32 rangeSqr = (5 * 24) * (5 * 24);

	if (Input::OnButtonDown(ONE)) { UI::MoveElement(hotbarHighlight, Vector2{ -equippedSlot * 0.04333333333f, 0.0f }); equippedSlot = 0; }
	else if (Input::OnButtonDown(TWO)) { UI::MoveElement(hotbarHighlight, Vector2{ (1.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 1; }
	else if (Input::OnButtonDown(THREE)) { UI::MoveElement(hotbarHighlight, Vector2{ (2.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 2; }
	else if (Input::OnButtonDown(FOUR)) { UI::MoveElement(hotbarHighlight, Vector2{ (3.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 3; }
	else if (Input::OnButtonDown(FIVE)) { UI::MoveElement(hotbarHighlight, Vector2{ (4.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 4; }
	else if (Input::OnButtonDown(SIX)) { UI::MoveElement(hotbarHighlight, Vector2{ (5.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 5; }
	else if (Input::OnButtonDown(SEVEN)) { UI::MoveElement(hotbarHighlight, Vector2{ (6.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 6; }
	else if (Input::OnButtonDown(EIGHT)) { UI::MoveElement(hotbarHighlight, Vector2{ (7.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 7; }
	else if (Input::OnButtonDown(NINE)) { UI::MoveElement(hotbarHighlight, Vector2{ (8.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 8; }
	else if (Input::OnButtonDown(ZERO)) { UI::MoveElement(hotbarHighlight, Vector2{ (9.0f - equippedSlot) * 0.04333333333f, 0.0f }); equippedSlot = 9; }
	else if (Input::MouseWheelDelta() != 0)
	{
		I16 delta = (I16)equippedSlot + Input::MouseWheelDelta();
		delta += 9 * (delta < 0) - 9 * (delta > 8);

		UI::MoveElement(hotbarHighlight, Vector2{ (delta - equippedSlot) * 0.04333333333f, 0.0f });
		equippedSlot = (U8)delta;
	}

	U16 id = (*hotbar)[equippedSlot][0].itemID;
	const Item* item = Items::GetItem(id);
	Vector2Int pos;

	switch (item->type)
	{
	case ITEM_TYPE_TILE: {
		const Block* block = (const Block*)item;

		if (Input::ButtonDown(LEFT_CLICK))
		{
			if (block->placeable && MouseToWorldInRange(pos, 5) && world->PlaceBlock(pos, (U8)id))
			{
				hotbar->RemoveItem(equippedSlot, 0, 1);
			}
		}
		else if(Input::ButtonDown(RIGHT_CLICK))
		{
			if (block->placeable && MouseToWorldInRange(pos, 5) && world->PlaceWall(pos, (U8)id))
			{
				hotbar->RemoveItem(equippedSlot, 0, 1);
			}
		}
	} break;
	case ITEM_TYPE_TOOL: {
		const Tool* tool = (const Tool*)item;

		if (Input::ButtonDown(LEFT_CLICK))
		{
			if (MouseToWorldInRange(pos, 5) &&
				Items::BlockToItem(world->tiles[pos.x][pos.y].blockID)->hardness <= tool->power &&
				Items::DecorationToItem(world->tiles[pos.x][pos.y].decID)->hardness <= tool->power)
			{
				world->BreakBlock(pos);
			}
		}
		else if (Input::ButtonDown(RIGHT_CLICK))
		{
			if (MouseToWorldInRange(pos, 5) &&
				Items::WallToItem(world->tiles[pos.x][pos.y].wallID)->hardness <= tool->power)
			{
				world->BreakWall(pos);
			}
		}
	} break;
	case ITEM_TYPE_WEAPON: {
		if (Input::ButtonDown(LEFT_CLICK) || Input::ButtonDown(RIGHT_CLICK))
		{
			player->Attack(((const Weapon*)item)->damage);
		}
	} break;
	case ITEM_TYPE_CONSUMABLE: {
		//TODO: Consume
	} break;
	case ITEM_TYPE_LIGHT: {
		const Light* light = (const Light*)item;

		if (Input::OnButtonDown(LEFT_CLICK))
		{
			if (MouseToWorldInRange(pos, 5) && world->PlaceLight(pos, light))
			{
				hotbar->RemoveItem(equippedSlot, 0, 1);
			}
		}
		else if (Input::OnButtonDown(RIGHT_CLICK))
		{
			if (MouseToWorldInRange(pos, 5))
			{
				world->RemoveLight(pos);
			}
		}
	} break;
	}
}

Vector2Int TimeSlip::MouseToWorld()
{
	Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
	Vector2 mousePos = (Vector2)Input::MousePos();
	Vector2 screenSize = (Vector2)Platform::ScreenSize();
	Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
	Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();
	Vector2 distance = mousePos - windowSize * 0.5f - windowOffset;

	return Vector2Int{ (distance / (windowSize.x * 0.0125f)) + cameraPos + 0.5f };
}

bool TimeSlip::MouseToWorldInRange(Vector2Int& pos, U8 range)
{
	Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
	Vector2 mousePos = (Vector2)Input::MousePos();
	Vector2 screenSize = (Vector2)Platform::ScreenSize();
	Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
	Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();
	Vector2 distance = mousePos - windowSize * 0.5f - windowOffset;

	pos = Vector2Int{ (distance / (windowSize.x * 0.0125f)) + cameraPos + 0.5f };

	return distance.SqrMagnitude() < (range * range * 576);
}

void TimeSlip::HandleEntities()
{
	U16 maxEntities = MAX_ENTITIES + ((MAX_ENTITIES >> 1) * (playerCount - 1));

	if (entities.Size() - playerCount < maxEntities && Math::RandomF() < ((F32)Time::DeltaTime() * 0.5f))
	{
		Vector2 spawn;
		Vector2 playerPos = player->gameObject->transform->Position();
		spawn.x = (U16)playerPos.x + 45.0f;

		if (spawn.x > world->TILES_X - 1) { spawn.x = (U16)playerPos.x - 45.0f; }

		for (U16 y = (U16)playerPos.y; y > 0; --y)
		{
			if (world->tiles[(U16)spawn.x][y].blockID == 0 && world->tiles[(U16)spawn.x][y - 1].blockID == 0)
			{
				spawn.y = y - 0.5f;
				break;
			}
		}

		EntityConfig eConfig{};
		eConfig.armor = 0.0f;
		eConfig.damageReduction = 0.0f;
		eConfig.knockbackReduction = 0.0f;
		eConfig.maxHealth = 100.0f;
		eConfig.position = spawn;
		eConfig.ignore = false;
		eConfig.despawnRange = 60.0f;
		eConfig.healthRegen = 0.0f;
		eConfig.color = { 0.5f, 1.0f, 0.5f };
		Enemy* enemy = new Enemy(eConfig, ENEMY_AI_BASIC);
		entities.Insert(enemy->gameObject->physics->ID(), enemy);
	}

	auto end = entities.end();
	for (auto it = entities.begin(); it != end;)
	{
		Entity* entity = *it;
		Vector2 pos = entity->gameObject->transform->Position();

		if ((pos - player->gameObject->transform->Position()).SqrMagnitude() > entity->despawnRange * entity->despawnRange && !entity->player)
		{
			entities.Remove(it, nullptr);
			entity->Destroy();
		}
		else
		{
			entity->Update();
			++it;
		}
	}
}

void TimeSlip::UpdateDayCycle()
{
	currentTime += (F32)Time::DeltaTime() * timeScale;

	F32 alpha = Math::Clamp(Math::Cos(currentTime * 0.01f) + 0.55f, 0.15f, 1.0f);

	bool night = alpha < 0.15f;

	globalColor = Vector3::ONE;

	worldScene->GetCamera()->SetAmbientColor(globalColor * alpha);
}

void TimeSlip::UpdateCraftingMenu()
{
	UI::DestroyAllChildren(ingredientList);

	UIElementConfig imageCfg{};
	imageCfg.position = { 0.0f, 0.0f };
	imageCfg.scale = { 0.23809523809f, 0.29714285714f };
	imageCfg.ignore = true;
	imageCfg.enabled = true;
	imageCfg.scene = worldScene;
	imageCfg.parent = ingredientList;

	UIElementConfig amtCfg{};
	amtCfg.position = { 0.0f, 0.0f };
	amtCfg.scale = { 1.0f, 1.0f };
	amtCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	amtCfg.ignore = true;
	amtCfg.enabled = true;
	amtCfg.scene = worldScene;

	bool found = true;

	for (U16 j = 0; j < selectedRecipe->ingredientCount; ++j)
	{
		if (inventory->ContainsItem(selectedRecipe->ingredients[j].id, selectedRecipe->ingredients[j].amount))
		{
			imageCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			amtCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		else
		{
			imageCfg.color = { 1.0f, 0.5f, 0.5f, 1.0f };
			amtCfg.color = { 1.0f, 0.5f, 0.5f, 1.0f };
			found = false;
		}

		amtCfg.parent = UI::GenerateImage(imageCfg, Resources::LoadTexture("Items.bmp"), Inventory::GetUV(selectedRecipe->ingredients[j].id));
		UI::GenerateText(amtCfg, selectedRecipe->ingredients[j].amount, 10);
		amtCfg.parent = nullptr;

		imageCfg.position.x += 0.25f;
	}

	UI::ChangeColor(craftButton, Vector4::ONE - Vector4{ 0.3f, 0.3f, 0.3f, 0.0f } * !found);
	if (found) { craftButton->OnClick = { OnClickCraft, (void*)selectedRecipe }; }
	else { craftButton->OnClick = { nullptr, nullptr }; }
	UI::SetEnable(craftButton, true);
}

void TimeSlip::PickupItem(U16 itemID, U16 amount)
{
	if (itemID > 0 && (hotbar->TryAddItem(itemID, amount) || inventory->AddItem(itemID, amount)) && selectedRecipe)
	{
		UpdateCraftingMenu();
	}
}

Transform2D* TimeSlip::GetTarget(Transform2D* position, F32 range)
{
	Vector2 pos = position->Position();

	Transform2D* bestTarget = nullptr;

	for (Entity* entity : entities)
	{
		if (entity->player && (entity->gameObject->transform->Position() - pos).Magnitude() < range)
		{
			bestTarget = entity->gameObject->transform;
		}
	}

	return bestTarget;
}

void TimeSlip::Attack(const Damage& damage, const Vector4& area)
{
	Box box{};
	box.xBounds = { area.x, area.z };
	box.yBounds = { area.y, area.w };

	Vector<PhysicsObject2D*> results;

	if (Physics::Query(box, results))
	{
		for (PhysicsObject2D* po : results)
		{
			Entity* e = entities[po->ID()];

			if (e && e->TakeDamage(damage) && e->Death())
			{
				entities.Remove(po->ID(), nullptr);
			}
		}
	}
}

void TimeSlip::CreateWorld(UIElement* element, const Vector2Int& mousePos, void* data)
{
	RendererFrontend::UseScene(worldScene);

	Vector2 spawnPoint;
	WorldSize size = *(WorldSize*)data;
	world = new World(10000000, size, spawnPoint);

	timeScale = 1.0f;
	currentTime = 1000.0f;
	night = false;

	EntityConfig eConfig{};
	eConfig.armor = 0.0f;
	eConfig.damageReduction = 0.0f;
	eConfig.knockbackReduction = 0.0f;
	eConfig.maxHealth = 100.0f;
	eConfig.position = spawnPoint;
	eConfig.ignore = false;
	eConfig.despawnRange = 40.0f;
	eConfig.healthRegen = 1.0f;
	eConfig.color = { 1.0f, 1.0f, 1.0f };
	player = new Player(eConfig);
	entities.Insert(player->gameObject->physics->ID(), player);

	worldScene->GetCamera()->SetPosition({ spawnPoint.x, spawnPoint.y, 10.0f });
	worldScene->GetCamera()->SetTarget(player->gameObject->transform);
	worldScene->GetCamera()->SetBounds({ 39.5f, size - 40.5f, 22.0f, size / 3.5f - 23.0f });

	CreateInventory();
	CreateCraftingMenu();

	nextState = GAME_STATE_GAME;
}

void TimeSlip::LoadWorld()
{

}

void TimeSlip::CreateMainMenu()
{
	UIElementConfig titleConfig{};
	titleConfig.position = { 0.25f, 0.05f };
	titleConfig.scale = { 0.5f, 0.2f };
	titleConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	titleConfig.enabled = true;
	titleConfig.ignore = true;
	titleConfig.scene = mainMenuScene;
	titleImage = UI::GeneratePanel(titleConfig, false);

	UIElementConfig playConfig{};
	playConfig.position = { 0.375f, 0.375f };
	playConfig.scale = { 0.25f, 0.1f };
	playConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	playConfig.enabled = true;
	playConfig.ignore = false;
	playConfig.scene = mainMenuScene;
	playButton = UI::GeneratePanel(playConfig, false);
	playButton->OnClick = { PlayMenu, nullptr };

	UIElementConfig settingsConfig{};
	settingsConfig.position = { 0.375f, 0.525f };
	settingsConfig.scale = { 0.25f, 0.1f };
	settingsConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	settingsConfig.enabled = true;
	settingsConfig.ignore = false;
	settingsConfig.scene = mainMenuScene;
	settingsButton = UI::GeneratePanel(settingsConfig, false);
	settingsButton->OnClick = { SettingsMenu, nullptr };

	UIElementConfig exitConfig{};
	exitConfig.position = { 0.375f, 0.675f };
	exitConfig.scale = { 0.25f, 0.1f };
	exitConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	exitConfig.enabled = true;
	exitConfig.ignore = false;
	exitConfig.scene = mainMenuScene;
	exitButton = UI::GeneratePanel(exitConfig, false);
	exitButton->OnClick = { Exit, nullptr };

	UIElementConfig smallWorldConfig{};
	smallWorldConfig.position = { 0.375f, 0.375f };
	smallWorldConfig.scale = { 0.25f, 0.1f };
	smallWorldConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	smallWorldConfig.enabled = false;
	smallWorldConfig.ignore = false;
	smallWorldConfig.scene = mainMenuScene;
	smallWorldButton = UI::GeneratePanel(smallWorldConfig, false);
	smallWorldButton->OnClick = { CreateWorld, (void*)&smallWorldSize };

	UIElementConfig mediumWorldConfig{};
	mediumWorldConfig.position = { 0.375f, 0.525f };
	mediumWorldConfig.scale = { 0.25f, 0.1f };
	mediumWorldConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	mediumWorldConfig.enabled = false;
	mediumWorldConfig.ignore = false;
	mediumWorldConfig.scene = mainMenuScene;
	mediumWorldButton = UI::GeneratePanel(mediumWorldConfig, false);
	mediumWorldButton->OnClick = { CreateWorld, (void*)&mediumWorldSize };

	UIElementConfig largeWorldConfig{};
	largeWorldConfig.position = { 0.375f, 0.675f };
	largeWorldConfig.scale = { 0.25f, 0.1f };
	largeWorldConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	largeWorldConfig.enabled = false;
	largeWorldConfig.ignore = false;
	largeWorldConfig.scene = mainMenuScene;
	largeWorldButton = UI::GeneratePanel(largeWorldConfig, false);
	largeWorldButton->OnClick = { CreateWorld, (void*)&largeWorldSize };

	UIElementConfig backConfig{};
	backConfig.position = { 0.375f, 0.825f };
	backConfig.scale = { 0.25f, 0.1f };
	backConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	backConfig.enabled = false;
	backConfig.ignore = false;
	backConfig.scene = mainMenuScene;
	backButton = UI::GeneratePanel(backConfig, false);
	backButton->OnClick = { MainMenu, nullptr };
}

void TimeSlip::CreateInventory()
{
	Inventory::Init(worldScene);

	InventoryConfig config{};
	config.color = Vector4{ 1.0f, 1.0f, 1.0f, 0.5f };
	config.slotColor = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	config.xMax = 9;
	config.yMax = 3;
	config.slotCount = 27;
	config.draggable = true;
	config.enable = true;
	config.startHorizontal = true;
	config.scene = worldScene;
	config.xPosition = 0.01f;
	config.yPosition = 0.025f;
	config.xSlotSize = 0.03333333333f;
	config.ySlotSize = 0.0624f;
	config.xPadding = 0.01f;
	config.xSpacing = 0.01f;
	config.yPadding = 0.0182f;
	config.ySpacing = 0.0182f;

	inventory = new Inventory(config);

	config.yMax = 1;
	config.slotCount = 9;
	config.xPosition = 0.3f;
	config.yPosition = 0.9f;

	hotbar = new Inventory(config);

	UIElementConfig highlightConfig{};
	highlightConfig.color = { 1.0f, 1.0f, 0.0f, 0.25f };
	highlightConfig.enabled = true;
	highlightConfig.ignore = true;
	highlightConfig.position = { 0.31f, 0.9182f };
	highlightConfig.scale = { 0.033333333333f, 0.0624f };
	highlightConfig.scene = worldScene;

	hotbarHighlight = UI::GeneratePanel(highlightConfig, false);
}

void TimeSlip::CreateCraftingMenu()
{
	UIElementConfig craftPanel{};
	craftPanel.color = { 1.0f, 1.0f, 1.0f, 0.5f };
	craftPanel.enabled = false;
	craftPanel.ignore = false;
	craftPanel.position = { 0.55f, 0.15f };
	craftPanel.scale = { 0.35f, 0.7f };
	craftPanel.scene = worldScene;

	craftingMenu = UI::GeneratePanel(craftPanel, true);
	craftingMenu->OnDrag = UI::OnDragDefault;

	UIElementConfig craftingImage{};
	craftingImage.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	craftingImage.enabled = true;
	craftingImage.ignore = true;
	craftingImage.position = { 0.55f, 0.1f };
	craftingImage.scale = { 0.35f, 0.31111111111f };
	craftingImage.scene = worldScene;
	craftingImage.parent = craftingMenu;

	craftResult = UI::GenerateImage(craftingImage, Resources::LoadTexture("Items.bmp"), blankUVs);

	UIElementConfig craftingButton{};
	craftingButton.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	craftingButton.enabled = false;
	craftingButton.ignore = false;
	craftingButton.position = { 0.55f, 0.8f };
	craftingButton.scale = { 0.3f, 0.09572649572f };
	craftingButton.scene = worldScene;
	craftingButton.parent = craftingMenu;

	Vector<Vector2> uvs{ 4 };
	uvs.Push({ 0.0f, 0.2f });
	uvs.Push({ 0.2f, 0.2f });
	uvs.Push({ 0.2f, 0.0f });
	uvs.Push({ 0.0f, 0.0f });

	craftButton = UI::GenerateImage(craftingButton, Resources::LoadTexture("TS_UI.bmp"), uvs);

	UIElementConfig ingrListCfg{};
	ingrListCfg.color = Vector4::ZERO;
	ingrListCfg.position = { 0.55f, 0.5f };
	ingrListCfg.scale = { 0.4f, 0.3f };
	ingrListCfg.ignore = true;
	ingrListCfg.enabled = true;
	ingrListCfg.scene = worldScene;
	ingrListCfg.parent = craftingMenu;

	ingredientList = UI::GeneratePanel(ingrListCfg, false);

	UIElementConfig recipeSelectCfg{};
	recipeSelectCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	recipeSelectCfg.enabled = false;
	recipeSelectCfg.ignore = true;
	recipeSelectCfg.position = { 0.0f, 0.0f };
	recipeSelectCfg.scale = { 0.3f, 0.26666666655f };
	recipeSelectCfg.scene = worldScene;
	recipeSelectCfg.parent = craftingMenu;

	Vector<Vector2> selectUvs{ 4 };
	selectUvs.Push({ 0.2f, 0.2f });
	selectUvs.Push({ 0.27179487179f, 0.2f });
	selectUvs.Push({ 0.27179487179f, 0.0f });
	selectUvs.Push({ 0.2f, 0.0f });

	recipeSelection = UI::GenerateImage(recipeSelectCfg, Resources::LoadTexture("TS_UI.bmp"), selectUvs);

	const Recipe** allRecipes = Items::GetRecipes();
	const Recipe* recipe = allRecipes[0];

	UIElementConfig recipeConfig{};
	recipeConfig.color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	recipeConfig.enabled = true;
	recipeConfig.ignore = false;
	recipeConfig.position = { 0.1f, 0.1f };
	recipeConfig.scale = { 0.2f, 0.1777777777f };
	recipeConfig.scene = worldScene;
	recipeConfig.parent = craftingMenu;

	U16 i = 0;
	while (recipe)
	{
		UIElement* e = UI::GenerateImage(recipeConfig, Resources::LoadTexture("Items.bmp"), Inventory::GetUV(recipe->result));
		e->OnClick = { OnClickSelect, (void*)recipe };

		recipeConfig.position.y += 0.3f;

		recipe = allRecipes[++i];
	}
}