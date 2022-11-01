#include "TimeSlip.hpp"

#include "Player.hpp"
#include "Inventory.hpp"
#include "Entity.hpp"
#include "Enemy.hpp"
#include "Tile.hpp"

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

#define MAX_ENTITIES 10

Scene* TimeSlip::mainMenuScene;
Scene* TimeSlip::worldScene;
World* TimeSlip::world;

F32 TimeSlip::timeScale;
F32 TimeSlip::currentTime;
bool TimeSlip::night;
Vector3 TimeSlip::globalColor;

Player* TimeSlip::player;
Inventory* TimeSlip::inventory;
Inventory* TimeSlip::hotbar;
UIElement* TimeSlip::hotbarHighlight;

U8 TimeSlip::equippedSlot;

HashTable<U64, Entity*> TimeSlip::entities;

U8 TimeSlip::playerCount;

GameState TimeSlip::gameState;
GameState TimeSlip::nextState;

WorldSize TimeSlip::testWorldSize{ WS_TEST };
WorldSize TimeSlip::smallWorldSize{ WS_SMALL };
WorldSize TimeSlip::mediumWorldSize{ WS_MEDIUM };
WorldSize TimeSlip::largeWorldSize{ WS_LARGE };

bool TimeSlip::Initialize()
{
	static const F32 camWidth = 3.63636363636f;
	static const F32 camHeight = 2.04545454545f;
	gameState = GAME_STATE_MENU;
	nextState = GAME_STATE_NONE;
	mainMenuScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	mainMenuScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	RendererFrontend::UseScene(mainMenuScene);

	worldScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	worldScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	worldScene->GetCamera()->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);

	//UI
	UIElementConfig createWorldConfig{};
	createWorldConfig.position = { 0.375f, 0.375f };
	createWorldConfig.scale = { 0.25f, 0.1f };
	createWorldConfig.color = { 0.0f, 0.7f, 1.0f, 1.0f };
	createWorldConfig.enabled = true;
	createWorldConfig.ignore = false;
	createWorldConfig.scene = mainMenuScene;
	UIElement* createWorldButton = UI::GeneratePanel(createWorldConfig, true);

	UIElementConfig generateWorldText{};
	generateWorldText.position = { 0.1f, -0.25f };
	generateWorldText.scale = { 0.9f, 0.9f };
	generateWorldText.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	generateWorldText.enabled = true;
	generateWorldText.ignore = true;
	generateWorldText.scene = mainMenuScene;
	generateWorldText.parent = createWorldButton;

	UI::GenerateText(generateWorldText, "Generate World", 20.0f);

	OnMouse createWorldEvent{};
	createWorldEvent.value = (void*)&testWorldSize;
	createWorldEvent.callback = CreateWorld;

	createWorldButton->OnClick = createWorldEvent;

	entities(19);

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

	return true;
}

void TimeSlip::HandleInput()
{
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
		delta += 9.0f * (delta < 0) - 9.0f * (delta > 8);

		UI::MoveElement(hotbarHighlight, Vector2{ (delta - equippedSlot) * 0.04333333333f, 0.0f }); 
		equippedSlot = delta;
	}

	if (Input::ButtonDown(LEFT_CLICK))
	{
		U16 itemID = (*hotbar)[equippedSlot][0].itemID;

		if (itemID == 22)
		{
			player->Attack();
		}
		else
		{
			Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
			Vector2 mousePos = (Vector2)Input::MousePos();
			Vector2 screenSize = (Vector2)Platform::ScreenSize();
			Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
			Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();
			Vector2 distance = mousePos - windowSize * 0.5f - windowOffset;

			if (distance.SqrMagnitude() < 20736.0f)
			{
				if (itemID < 11)
				{
					world->PlaceBlock(Vector2Int{ (distance / (windowSize.x * 0.0125f)) + cameraPos + 0.5f }, itemID);
				}
				else if (itemID == 21)
				{
					world->BreakBlock(Vector2Int{ (distance / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
				}
			}
		}
	}

	if (Input::ButtonDown(RIGHT_CLICK))
	{
		U16 itemID = (*hotbar)[equippedSlot][0].itemID;

		if (itemID == 22)
		{
			player->Attack();
		}
		else
		{
			Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
			Vector2 mousePos = (Vector2)Input::MousePos();
			Vector2 screenSize = (Vector2)Platform::ScreenSize();
			Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
			Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();
			Vector2 distance = mousePos - windowSize * 0.5f - windowOffset;

			if (distance.SqrMagnitude() < 20736.0f)
			{
				if (itemID < 11)
				{
					world->PlaceWall(Vector2Int{ (distance / (windowSize.x * 0.0125f)) + cameraPos + 0.5f }, itemID);
				}
				else if (itemID == 21)
				{
					world->BreakWall(Vector2Int{ (distance / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
				}
			}
		}
	}
}

void TimeSlip::HandleEntities()
{
	U16 maxEntities = MAX_ENTITIES + ((MAX_ENTITIES >> 1) * (playerCount - 1));

	if (entities.Size() - playerCount < maxEntities && Math::RandomF() < ((F32)Time::DeltaTime() * 0.5f))
	{
		Vector2 spawn;
		Vector2 playerPos = player->gameObject->transform->Position();
		spawn.x = (U16)playerPos.x + 45.0f;

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
		eConfig.regeneration = 0.0f;
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

	F32 alpha = Math::Clamp(Math::Cos(currentTime * 0.1f) + 0.55f, 0.15f, 1.0f);

	bool night = alpha < 0.15f;

	globalColor = Vector3::ONE;

	worldScene->GetCamera()->SetAmbientColor(globalColor * alpha);
}

void TimeSlip::LoadWorld()
{

}

void TimeSlip::PickupItem(U16 itemID, U16 amount)
{
	inventory->AddItem(itemID, amount);
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
	eConfig.regeneration = 1.0f;
	player = new Player(eConfig);
	entities.Insert(player->gameObject->physics->ID(), player);

	worldScene->GetCamera()->SetPosition({ spawnPoint.x, spawnPoint.y, 10.0f });
	worldScene->GetCamera()->SetTarget(player->gameObject->transform);
	worldScene->GetCamera()->SetBounds({ 39.5f, size - 40.5f, 22.0f, size / 3.5f - 23.0f });

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

	hotbar->AddItem(22, 1);
	hotbar->AddItem(21, 1);

	nextState = GAME_STATE_GAME;
}