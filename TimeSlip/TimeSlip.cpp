#include "TimeSlip.hpp"

#include "Player.hpp"
#include "Inventory.hpp"

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

Scene* TimeSlip::mainMenuScene;
Scene* TimeSlip::worldScene;
World* TimeSlip::world;

F32 TimeSlip::timeScale;
F32 TimeSlip::currentTime;
bool TimeSlip::night;
Vector3 TimeSlip::globalColor;

Player* TimeSlip::player;
Inventory* TimeSlip::inventory;

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

	return true;
}

void TimeSlip::Shutdown()
{
	if (player) { player->Destroy(); }
	if (world) { world->Destroy(); }
}

bool TimeSlip::Update()
{
	switch (gameState)
	{
	case GAME_STATE_MENU: break;
	case GAME_STATE_GAME:
		Inventory::Update();
		player->Update();
		worldScene->GetCamera()->Update();
		world->Update();
		UpdateDayCycle();

		if (Input::OnButtonDown(I)) { inventory->ToggleShow(); }
		break;
	case GAME_STATE_PAUSE: break;
		//TODO: Pause Physics
	default: break;
	}

	if (nextState) { gameState = nextState; nextState = GAME_STATE_NONE; }

	return true;
}

void TimeSlip::UpdateDayCycle()
{
	currentTime += Time::DeltaTime() * timeScale;

	F32 alpha = Math::Clamp(Math::Cos(currentTime * 0.1f) + 0.55f, 0.1f, 1.0f);

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

void TimeSlip::CreateWorld(UIElement* element, const Vector2Int& mousePos, void* data)
{
	RendererFrontend::UseScene(worldScene);

	Vector2 spawnPoint;
	WorldSize size = *(WorldSize*)data;
	world = new World(10000000, size, spawnPoint);

	timeScale = 1.0f;
	currentTime = 1000.0f;
	night = false;

	player = new Player(spawnPoint);
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
	config.height = 0.26f;
	config.width = 0.4f;
	config.scene = worldScene;
	config.xSlotSize = 0.08333333333;
	config.ySlotSize = 0.24f;
	config.xPadding = 0.025f;
	config.xSpacing = 0.025f;
	config.yPadding = 0.07f;
	config.ySpacing = 0.07f;

	inventory = new Inventory(config);

	nextState = GAME_STATE_GAME;
}