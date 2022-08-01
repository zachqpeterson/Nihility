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

bool init()
{
	scene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	scene->Create(CAMERA_TYPE_ORTHOGRAPHIC);

	MeshConfig backgroundConfig;
	backgroundConfig.name = "Background";
	backgroundConfig.MaterialName = "Background.mat";

	backgroundConfig.vertices.Push(Vertex{ {-1.0f, -1.0f, 0.0f}, { 0.0f, 0.0f } });
	backgroundConfig.vertices.Push(Vertex{ { 1.0f, -1.0f, 0.0f}, { 1.0f, 0.0f } });
	backgroundConfig.vertices.Push(Vertex{ { 1.0f,  1.0f, 0.0f}, { 1.0f, 1.0f } });
	backgroundConfig.vertices.Push(Vertex{ {-1.0f,  1.0f, 0.0f}, { 0.0f, 1.0f } });

	backgroundConfig.indices.Push(0);
	backgroundConfig.indices.Push(1);
	backgroundConfig.indices.Push(2);
	backgroundConfig.indices.Push(2);
	backgroundConfig.indices.Push(3);
	backgroundConfig.indices.Push(0);

	Mesh* backgroundMesh = Resources::CreateMesh(backgroundConfig);

	MeshConfig config0;
	config0.name = "Mesh0";
	config0.MaterialName = "Tile.mat";
	config0.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

	config0.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
	config0.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
	config0.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
	config0.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

	config0.indices.Push(0);
	config0.indices.Push(1);
	config0.indices.Push(2);
	config0.indices.Push(2);
	config0.indices.Push(3);
	config0.indices.Push(0);

	Mesh* mesh0 = Resources::CreateMesh(config0);
	Transform2D* transform = new Transform2D();
	transform->Translate({ 0.0f, -100.0f });
	transform->SetScale({ 1.0f, 1.0f });
	PhysicsObject2DConfig poConfig{};
	poConfig.density = 1.0f;
	poConfig.dragCoefficient = 2.0f;
	poConfig.gravityScale = 1.0f;
	poConfig.kinematic = false;
	poConfig.restitution = 0.5f;
	poConfig.transform = transform;
	poConfig.trigger = false;
	poConfig.type = COLLIDER_TYPE_RECTANGLE;
	poConfig.xBounds = { -0.5f, 0.5f };
	poConfig.yBounds = { -0.5f, 0.5f };
	Vector<Mesh*> meshes(1, mesh0);
	GameObject2DConfig goConfig{};
	goConfig.name = "Mesh0";
	goConfig.transform = transform;
	goConfig.model = Resources::CreateModel("Mesh0", meshes);
	goConfig.physics = Physics::Create2DPhysicsObject(poConfig);
	GameObject2D* gameObject = Resources::CreateGameObject2D(goConfig);

	//FLOOR
	MeshConfig config1;
	config1.name = "Mesh1";
	config1.MaterialName = "Tile.mat";
	config1.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));

	config1.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f } });
	config1.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f } });
	config1.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f } });
	config1.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f } });

	config1.indices.Push(0);
	config1.indices.Push(1);
	config1.indices.Push(2);
	config1.indices.Push(2);
	config1.indices.Push(3);
	config1.indices.Push(0);

	Mesh* mesh1 = Resources::CreateMesh(config1);
	Transform2D* transform1 = new Transform2D();
	transform1->Translate({ 0.0f, 10.0f });
	transform1->SetScale({ 10.0f, 10.0f });
	PhysicsObject2DConfig poConfig1{};
	poConfig1.density = 0.0f;
	poConfig1.dragCoefficient = 0.0f;
	poConfig1.gravityScale = 1.0f;
	poConfig1.kinematic = true;
	poConfig1.restitution = 0.0f;
	poConfig1.transform = transform1;
	poConfig1.trigger = false;
	poConfig1.type = COLLIDER_TYPE_RECTANGLE;
	poConfig1.xBounds = { -5.0f, 5.0f };
	poConfig1.yBounds = { -5.0f, 5.0f };
	Vector<Mesh*> meshes1(1, mesh1);
	GameObject2DConfig goConfig1{};
	goConfig1.name = "Mesh1";
	goConfig1.transform = transform1;
	goConfig1.model = Resources::CreateModel("Mesh1", meshes1);
	goConfig1.physics = Physics::Create2DPhysicsObject(poConfig1);
	GameObject2D* gameObject1 = Resources::CreateGameObject2D(goConfig1);

	//TEXT
	UIElementConfig config{};
	config.area = { 0.0f, 0.9f, 0.1f, 1.0f };
	config.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.name = "Text0";
	config.scene = scene;
	
	UI::GenerateText(config, "Hello, World!");

	//scene->DrawMesh(backgroundMesh);
	scene->DrawGameObject(gameObject);
	scene->DrawGameObject(gameObject1);

	RendererFrontend::UseScene(scene);

	return true;
}

bool update()
{
	Vector3 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), (F32)(Input::ButtonDown(S) - Input::ButtonDown(W)), (F32)(Input::ButtonDown(E) - Input::ButtonDown(Q)) };
	move *= Time::DeltaTime() * 10.0;
	scene->GetCamera()->Translate(move);

	return true;
}

void cleanup()
{

}

int main()
{
	Engine::Initialize("TestBed", init, update, cleanup);
}