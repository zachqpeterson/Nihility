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

Scene* scene;
GameObject2D* player;
Model* dirtModel;

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
	poConfig.restitution = 0.0;
	poConfig.transform = transform;
	poConfig.trigger = false;
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

	PhysicsObject2DConfig floorConfig{};
	floorConfig.density = 0.0;
	floorConfig.gravityScale = 1.0;
	floorConfig.kinematic = true;
	floorConfig.restitution = 0.0;
	floorConfig.trigger = false;
	floorConfig.type = BOX_COLLIDER;
	floorConfig.box = { {-0.5f, 0.5f}, {-0.5f, 0.5f} };

	String name{ "Floor" };

	for (I32 i = -5; i < 6; ++i)
	{
		Transform2D* transform = new Transform2D();
		transform->Translate({ (F32)i, 10.0f });
		transform->SetScale({ 1.0f, 1.0f });

		floorConfig.transform = transform;

		GameObject2DConfig goConfig{};
		goConfig.name = name + (i + 5);
		goConfig.transform = transform;
		goConfig.model = dirtModel;
		goConfig.physics = Physics::Create2DPhysicsObject(floorConfig);
		GameObject2D* gameObject = Resources::CreateGameObject2D(goConfig);
		scene->DrawGameObject(gameObject);
	}

	Transform2D* transform = new Transform2D();
	transform->Translate({ 0.6f, 0.0f });
	PhysicsObject2DConfig poConfig{};
	poConfig.density = 1.0;
	poConfig.gravityScale = 1.0;
	poConfig.kinematic = false;
	poConfig.restitution = 0.0;
	poConfig.transform = transform;
	poConfig.trigger = false;
	poConfig.type = BOX_COLLIDER;
	poConfig.box = { {-0.5f, 0.5f}, {-0.5f, 0.5f} };
	poConfig.radius = 0.5;
	poConfig.shape.Reserve(4);
	poConfig.shape.Push({ -0.5f, -0.5f });
	poConfig.shape.Push({ -0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f, -0.5f });

	GameObject2DConfig goConfig{};
	goConfig.name = "Player";
	goConfig.transform = transform;
	goConfig.model = dirtModel;
	goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
	//player = Resources::CreateGameObject2D(goConfig);
	//scene->DrawGameObject(player);

	//PANEL
	UIElementConfig panelCfg{};
	panelCfg.area = { 0.01f, 0.02f, 0.41f, 0.28f };
	panelCfg.color = { 0.0f, 0.0f, 1.0f, 1.0f };
	panelCfg.enabled = true;
	panelCfg.name = "Panel1";
	panelCfg.scene = scene;
	UI::GenerateBorderedPanel(panelCfg);

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
			slotCfg.area = { x, y, x + width, y + height };
			slotCfg.color = { 1.0f, 0.0f, 0.0f, 1.0f };
			slotCfg.enabled = true;
			slotCfg.name = slotName + (i + j * xAmt);
			slotCfg.parentName = "Panel1";
			slotCfg.scene = scene;

			UI::GeneratePanel(slotCfg);
		}
	}

	//TEXT
	UIElementConfig config{};
	config.area = { 0.0f, 0.9f, 0.1f, 1.0f };
	config.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.name = "Text0";
	config.scene = scene;

	UI::GenerateText(config, "Hello, World!");

	RendererFrontend::UseScene(scene);

	return true;
}

bool update()
{
	//Vector2 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)) };
	//move *= (F32)(Time::DeltaTime());

	if (Input::ButtonDown(D))
	{
		player->physics->Translate({ 10.0f * (F32)Time::DeltaTime(), 0.0f });
	}

	if (Input::ButtonDown(SPACE) && player->physics->Grounded())
	{
		player->physics->ApplyForce({ 0.0f, -1.0f });
	}

	if (Input::OnButtonDown(LBUTTON))
	{
		spawnObj(RendererFrontend::ScreenToWorld(Input::MousePos()));
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