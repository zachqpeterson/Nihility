#include <Engine.hpp>

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Math/Math.hpp>
#include <Core/Logger.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Resources/Resources.hpp>
#include <Resources/UI.hpp>
#include <Physics/Physics.hpp>
#include <Audio/Audio.hpp>

//#include "GridBroadphase.hpp"

Scene* scene;
GameObject2D* player;
Model* dirtModel;

void OnClick(UIElement* e, const Vector2Int& pos)
{

}

void OnDrag(UIElement* e, const Vector2Int& delta)
{
	UI::MoveElement(e, delta);
}

void OnRelease(UIElement* e, const Vector2Int& pos)
{

}

void OnHover(UIElement* e, const Vector2Int& pos)
{
	UI::ChangeColor(e, { 0.8f, 0.0f, 0.0f, 1.0f });
	UI::ShowDescription(pos);
}

void OnMove(UIElement* e, const Vector2Int& pos)
{
	UI::MoveDescription(pos);
}

void OnExit(UIElement* e)
{
	UI::ChangeColor(e, { 1.0f, 0.0f, 0.0f, 1.0f });
	UI::HideDescription();
}

void OnScrll(UIElement* e, const Vector2Int& pos, I16 delta)
{
	Logger::Debug("Scroll: {}", delta);
}

void spawnObj(const Vector2& position)
{
	static U32 id = 0;

	String name("go{}", id);
	Transform2D* transform = new Transform2D();
	transform->SetPosition(position);
	PhysicsObject2DConfig poConfig{};
	poConfig.density = 1.0;
	poConfig.gravityScale = 1.0;
	poConfig.kinematic = false;
	poConfig.friction = 0.2f;
	poConfig.restitution = 0.0;
	poConfig.transform = transform;
	poConfig.trigger = false;
	poConfig.freezeRotation = false;
	poConfig.layerMask = 1;
	poConfig.type = BOX_COLLIDER;
	poConfig.box = { {-0.5f, 0.5f}, {-0.5f, 0.5f} };
	poConfig.radius = 0.5;
	poConfig.shape.Reserve(4);
	poConfig.shape.Push({ -0.5f, -0.5f });
	poConfig.shape.Push({ -0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f, -0.5f });

	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.transform = transform;
	goConfig.model = dirtModel;
	goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
	scene->DrawGameObject(Resources::CreateGameObject2D(goConfig));

	++id;
}

bool init()
{
	scene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	scene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	scene->GetCamera()->SetPosition({ 55.0f, 90.0f, 10.0f });

	//GridBroadphase* bp = new GridBroadphase(5600, 1600);
	//Physics::SetBroadphase(bp);

	MeshConfig dirtConfig;
	dirtConfig.name = "Dirt";
	dirtConfig.MaterialName = "Tile.mat";
	dirtConfig.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

	dirtConfig.vertices.Reserve(4);
	dirtConfig.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
	dirtConfig.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
	dirtConfig.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
	dirtConfig.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

	dirtConfig.indices.Reserve(6);
	dirtConfig.indices.Push(0);
	dirtConfig.indices.Push(1);
	dirtConfig.indices.Push(2);
	dirtConfig.indices.Push(2);
	dirtConfig.indices.Push(3);
	dirtConfig.indices.Push(0);

	Mesh* dirtMesh = Resources::CreateMesh(dirtConfig);
	Vector<Mesh*> meshes(1, dirtMesh);
	dirtModel = Resources::CreateModel("Dirt", meshes);

	String name{ "Floor" };

	for (I32 i = 50; i < 60; ++i)
	{
		Transform2D* transform = new Transform2D();
		transform->Translate({ (F32)i, 100.0f });
		transform->SetScale({ 1.0f, 1.0f });

		GameObject2DConfig goConfig{};
		goConfig.name = name + (i + 5);
		goConfig.transform = transform;
		goConfig.model = dirtModel;

		//bp->ChangeTile(i, 100, 1);

		GameObject2D* gameObject = Resources::CreateGameObject2D(goConfig);
		scene->DrawGameObject(gameObject);
	}

	Transform2D* wallTransform0 = new Transform2D();
	wallTransform0->Translate({ 50.0f, 99.0f });
	wallTransform0->SetScale({ 1.0f, 1.0f });

	GameObject2DConfig wallConfig0{};
	wallConfig0.name = name + (11 + 5);
	wallConfig0.transform = wallTransform0;
	wallConfig0.model = dirtModel;

	//bp->ChangeTile(50, 99, 1);

	GameObject2D* wall0 = Resources::CreateGameObject2D(wallConfig0);
	scene->DrawGameObject(wall0);

	Transform2D* wallTransform1 = new Transform2D();
	wallTransform1->Translate({ 59.0f, 99.0f });
	wallTransform1->SetScale({ 1.0f, 1.0f });

	GameObject2DConfig wallConfig1{};
	wallConfig1.name = name + (12 + 5);
	wallConfig1.transform = wallTransform1;
	wallConfig1.model = dirtModel;

	//bp->ChangeTile(59, 99, 1);

	GameObject2D* wall1 = Resources::CreateGameObject2D(wallConfig1);
	scene->DrawGameObject(wall1);

	Transform2D* wallTransform2 = new Transform2D();
	wallTransform2->Translate({ 55.0f, 98.0f });
	wallTransform2->SetScale({ 1.0f, 1.0f });

	GameObject2DConfig wallConfig2{};
	wallConfig2.name = name + (13 + 5);
	wallConfig2.transform = wallTransform2;
	wallConfig2.model = dirtModel;

	//bp->ChangeTile(55, 98, 1);

	GameObject2D* wall2 = Resources::CreateGameObject2D(wallConfig2);
	scene->DrawGameObject(wall2);

	/*Transform2D* wallTransform3 = new Transform2D();
	wallTransform3->Translate({ 54.0f, 97.0f });
	wallTransform3->SetScale({ 1.0f, 1.0f });

	GameObject2DConfig wallConfig3{};
	wallConfig3.name = name + (14 + 5);
	wallConfig3.transform = wallTransform3;
	wallConfig3.model = dirtModel;

	bp->ChangeTile(54, 97, 1);

	GameObject2D* wall3 = Resources::CreateGameObject2D(wallConfig3);
	scene->DrawGameObject(wall3);

	Transform2D* wallTransform4 = new Transform2D();
	wallTransform4->Translate({ 56.0f, 97.0f });
	wallTransform4->SetScale({ 1.0f, 1.0f });

	GameObject2DConfig wallConfig4{};
	wallConfig4.name = name + (15 + 5);
	wallConfig4.transform = wallTransform4;
	wallConfig4.model = dirtModel;

	bp->ChangeTile(56, 97, 1);

	GameObject2D* wall4 = Resources::CreateGameObject2D(wallConfig4);
	scene->DrawGameObject(wall4);*/

	Transform2D* transform = new Transform2D();
	transform->Translate({ 55.0f, 90.0f });
	transform->SetScale({ 1.0f, 2.0f });
	PhysicsObject2DConfig poConfig{};
	poConfig.density = 0.5;
	poConfig.gravityScale = 1.0;
	poConfig.kinematic = false;
	poConfig.restitution = 0.0;
	poConfig.friction = 0.2f;
	poConfig.transform = transform;
	poConfig.trigger = false;
	poConfig.freezeRotation = true;
	poConfig.layerMask = 1;
	poConfig.type = BOX_COLLIDER;
	poConfig.box = { {-0.5f, 0.5f}, {-1.0f, 1.0f} };

	GameObject2DConfig goConfig{};
	goConfig.name = "Player";
	goConfig.transform = transform;
	goConfig.model = dirtModel;
	goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
	player = Resources::CreateGameObject2D(goConfig);
	scene->DrawGameObject(player);
	Audio::SetListener(player->transform);

	//PANEL
	UIElementConfig panelCfg{};
	panelCfg.position = { 0.01f, 0.05f };
	panelCfg.scale = { 0.4f, 0.26f };
	panelCfg.color = { 0.0f, 0.0f, 1.0f, 1.0f };
	panelCfg.enabled = true;
	panelCfg.scene = scene;
	UIElement* panel = UI::GeneratePanel(panelCfg, true);

	panel->OnDrag = OnDrag;

	//SLOTS
	String slotName("Slot");
	U32 xAmt = 9;
	F32 xGap = 0.025f;
	F32 width = (1.0f - xGap * (xAmt + 1)) / xAmt;
	U32 yAmt = 3;
	F32 yGap = 0.07f;
	F32 height = (1.0f - yGap * (yAmt + 1)) / yAmt;

	for (U32 i = 0; i < xAmt; ++i)
	{
		for (U32 j = 0; j < yAmt; ++j)
		{
			UIElementConfig slotCfg{};
			F32 x = xGap + ((width + xGap) * i);
			F32 y = yGap + ((height + yGap) * j);
			slotCfg.position = { x, y };
			slotCfg.scale = { width, height };
			slotCfg.color = { 1.0f, 0.0f, 0.0f, 1.0f };
			slotCfg.enabled = true;
			slotCfg.parent = panel;
			slotCfg.scene = scene;

			UIElement* slot = UI::GeneratePanel(slotCfg, false);
			slot->OnClick = OnClick;
			slot->OnRelease = OnRelease;
			slot->OnHover = OnHover;
			slot->OnMove = OnMove;
			slot->OnExit = OnExit;
			slot->OnScroll = OnScrll;
		}
	}

	//TEXT
	UIElementConfig config{};
	config.position = { 0.0f, 0.9f };
	config.scale = { 0.1f, 0.1f };
	config.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.scene = scene;

	UI::GenerateText(config, "Hello, World!", 72.0f);

	RendererFrontend::UseScene(scene);

	return true;
}

bool update()
{
	Vector2 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), 0.0f };
	move *= (F32)(Time::DeltaTime());

	player->physics->Translate(move * 10.0f);

	player->physics->SetGravityScale(0.5f + 0.5f * !Input::ButtonDown(SPACE));

	if (Input::ButtonDown(SPACE) && player->physics->Grounded())
	{
		player->physics->ApplyForce({ 0.0f, -1.0f });
	}

	if (Input::OnButtonDown(LBUTTON))
	{
		spawnObj(RendererFrontend::ScreenToWorld((Vector2)Input::MousePos()));
	}

	return true;
}

void cleanup()
{

}

int main()
{
	Engine::Initialize("Nihility Demo", init, update, cleanup);
}