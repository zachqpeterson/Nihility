#include "TimeSlip.hpp"

#include "Player.hpp"
#include "Inventory.hpp"
#include "Entity.hpp"
#include "Enemy.hpp"

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
Inventory* TimeSlip::hotBar;

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

void TimeSlip::HandleEntities()
{
	U16 maxEntities = MAX_ENTITIES + ((MAX_ENTITIES >> 1) * (playerCount - 1));

	if (entities.Size() - playerCount < maxEntities)
	{
		//TODO: Spawn Entities
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

Transform2D* TimeSlip::GetTarget(Transform2D* position)
{
	Vector2 pos = position->Position();

	Transform2D* bestTarget = nullptr;

	F32 maxDistance = 10.0f;

	for (Entity* entity : entities)
	{
		if (entity->player && (entity->gameObject->transform->Position() - pos).Magnitude() < maxDistance)
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
	worldScene->GetCamera()->SetBounds({39.5f, size - 40.5f, 22.0f, size / 3.5f - 23.0f});

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

	hotBar = new Inventory(config);

	nextState = GAME_STATE_GAME;
}