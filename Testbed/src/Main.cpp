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
Model* model;

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
	poConfig.type = POLYGON_COLLIDER;
	poConfig.radius = 0.5;
	poConfig.shape.Reserve(4);
	poConfig.shape.Push({ -0.5f, -0.5f });
	poConfig.shape.Push({ -0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f, -0.5f });

	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.transform = transform;
	goConfig.model = model;
	goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
	scene->DrawGameObject(Resources::CreateGameObject2D(goConfig));

	++id;
}

bool init()
{
	scene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	scene->Create(CAMERA_TYPE_ORTHOGRAPHIC);

	{
		MeshConfig config;
		config.name = "Mesh";
		config.MaterialName = "Tile.mat";
		config.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

		config.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
		config.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

		config.indices.Push(0);
		config.indices.Push(1);
		config.indices.Push(2);
		config.indices.Push(2);
		config.indices.Push(3);
		config.indices.Push(0);

		Mesh* mesh = Resources::CreateMesh(config);
		Vector<Mesh*> meshes(1, mesh);
		model = Resources::CreateModel("Model", meshes);
	}

	//FLOOR
	{
		MeshConfig config;
		config.name = "Floor0";
		config.MaterialName = "Tile.mat";
		config.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

		config.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
		config.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

		config.indices.Push(0);
		config.indices.Push(1);
		config.indices.Push(2);
		config.indices.Push(2);
		config.indices.Push(3);
		config.indices.Push(0);

		Mesh* mesh = Resources::CreateMesh(config);
		Transform2D* transform = new Transform2D();
		transform->Translate({ 0.0f, 10.0f });
		transform->SetScale({ 10.0f, 10.0f });
		PhysicsObject2DConfig poConfig{};
		poConfig.density = 0.0;
		poConfig.gravityScale = 1.0;
		poConfig.kinematic = true;
		poConfig.restitution = 0.0;
		poConfig.transform = transform;
		poConfig.trigger = false;
		poConfig.type = POLYGON_COLLIDER;
		poConfig.radius = 5.0;
		poConfig.shape.Reserve(4);
		poConfig.shape.Push({ -5.0f, -5.0f });
		poConfig.shape.Push({ -5.0f,  5.0f });
		poConfig.shape.Push({ 5.0f,  5.0f });
		poConfig.shape.Push({ 5.0f, -5.0f });

		Vector<Mesh*> meshes(1, mesh);
		GameObject2DConfig goConfig{};
		goConfig.name = "Floor0";
		goConfig.transform = transform;
		goConfig.model = Resources::CreateModel("Floor0", meshes);
		goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
		GameObject2D* gameObject = Resources::CreateGameObject2D(goConfig);
		scene->DrawGameObject(gameObject);
	}
	{
		MeshConfig config;
		config.name = "Floor1";
		config.MaterialName = "Tile.mat";
		config.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

		config.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
		config.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

		config.indices.Push(0);
		config.indices.Push(1);
		config.indices.Push(2);
		config.indices.Push(2);
		config.indices.Push(3);
		config.indices.Push(0);

		Mesh* mesh = Resources::CreateMesh(config);
		Transform2D* transform = new Transform2D();
		transform->Translate({ 10.0f, 0.0f });
		transform->SetScale({ 10.0f, 10.0f });
		PhysicsObject2DConfig poConfig{};
		poConfig.density = 0.0;
		poConfig.gravityScale = 1.0;
		poConfig.kinematic = true;
		poConfig.restitution = 0.0;
		poConfig.transform = transform;
		poConfig.trigger = false;
		poConfig.type = POLYGON_COLLIDER;
		poConfig.radius = 5.0;
		poConfig.shape.Reserve(4);
		poConfig.shape.Push({ -5.0f, -5.0f });
		poConfig.shape.Push({ -5.0f,  5.0f });
		poConfig.shape.Push({ 5.0f,  5.0f });
		poConfig.shape.Push({ 5.0f, -5.0f });

		Vector<Mesh*> meshes(1, mesh);
		GameObject2DConfig goConfig{};
		goConfig.name = "Floor1";
		goConfig.transform = transform;
		goConfig.model = Resources::CreateModel("Floor1", meshes);
		goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
		GameObject2D* gameObject = Resources::CreateGameObject2D(goConfig);
		scene->DrawGameObject(gameObject);
	}
	{
		MeshConfig config;
		config.name = "Floor2";
		config.MaterialName = "Tile.mat";
		config.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

		config.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
		config.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
		config.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

		config.indices.Push(0);
		config.indices.Push(1);
		config.indices.Push(2);
		config.indices.Push(2);
		config.indices.Push(3);
		config.indices.Push(0);

		Mesh* mesh = Resources::CreateMesh(config);
		Transform2D* transform = new Transform2D();
		transform->Translate({ -10.0f, 0.0f });
		transform->SetScale({ 10.0f, 10.0f });
		PhysicsObject2DConfig poConfig{};
		poConfig.density = 0.0;
		poConfig.gravityScale = 1.0;
		poConfig.kinematic = true;
		poConfig.restitution = 0.0;
		poConfig.transform = transform;
		poConfig.trigger = false;
		poConfig.type = POLYGON_COLLIDER;
		poConfig.radius = 5.0;
		poConfig.shape.Reserve(4);
		poConfig.shape.Push({ -5.0f, -5.0f });
		poConfig.shape.Push({ -5.0f,  5.0f });
		poConfig.shape.Push({ 5.0f,  5.0f });
		poConfig.shape.Push({ 5.0f, -5.0f });

		Vector<Mesh*> meshes(1, mesh);
		GameObject2DConfig goConfig{};
		goConfig.name = "Floor2";
		goConfig.transform = transform;
		goConfig.model = Resources::CreateModel("Floor2", meshes);
		goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
		GameObject2D* gameObject = Resources::CreateGameObject2D(goConfig);
		scene->DrawGameObject(gameObject);
	}

	String name("player");
	Transform2D* transform = new Transform2D();
	transform->Translate({6.1f, -8.3f});
	PhysicsObject2DConfig poConfig{};
	poConfig.density = 1.0;
	poConfig.gravityScale = 1.0;
	poConfig.kinematic = false;
	poConfig.restitution = 0.0;
	poConfig.transform = transform;
	poConfig.trigger = false;
	poConfig.type = POLYGON_COLLIDER;
	poConfig.radius = 0.5;
	poConfig.shape.Reserve(4);
	poConfig.shape.Push({ -0.5f, -0.5f });
	poConfig.shape.Push({ -0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f,  0.5f });
	poConfig.shape.Push({ 0.5f, -0.5f });

	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.transform = transform;
	goConfig.model = model;
	goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
	player = Resources::CreateGameObject2D(goConfig);
	scene->DrawGameObject(player);

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
	Vector2 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)) };
	move *= (F32)(Time::DeltaTime() * 3.0);

	player->physics->Translate(move);

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
	Engine::Initialize("TestBed", init, update, cleanup);
}