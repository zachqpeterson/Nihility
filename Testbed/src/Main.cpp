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

	MeshConfig config0;
	config0.name = "Mesh0";
	config0.MaterialName = "Tile.mat";

	config0.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.33333333333f, 0.625f } });
	config0.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.5f, 0.625f } });
	config0.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.5f, 0.5f } });
	config0.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.33333333333f, 0.5f } });

	config0.indices.Push(0);
	config0.indices.Push(1);
	config0.indices.Push(2);
	config0.indices.Push(2);
	config0.indices.Push(3);
	config0.indices.Push(0);

	MeshConfig config1;
	config1.name = "Mesh1";
	config1.MaterialName = "Tile.mat";

	config1.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.375f } });
	config1.vertices.Push(Vertex{ { 1.5f, -0.5f, 0.0f}, { 0.33333333333f, 0.375f } });
	config1.vertices.Push(Vertex{ { 1.5f,  0.5f, 0.0f}, { 0.33333333333f, 0.25f } });
	config1.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.25f } });

	config1.indices.Push(0);
	config1.indices.Push(1);
	config1.indices.Push(2);
	config1.indices.Push(2);
	config1.indices.Push(3);
	config1.indices.Push(0);

	Mesh* backgroundMesh = Resources::CreateMesh(backgroundConfig);
	Mesh* mesh0 = Resources::CreateMesh(config0);
	Mesh* mesh1 = Resources::CreateMesh(config1);
	Panel* panel = UI::GeneratePanel({ 0.0f, 1.0f, 1.0f, 0.0f });
	scene->DrawMesh(backgroundMesh, Matrix4::IDENTITY);
	scene->DrawMesh(mesh0, Matrix4::IDENTITY);
	scene->DrawMesh(mesh1, Matrix4::IDENTITY);
	scene->DrawMesh(panel->mesh, Matrix4::IDENTITY);

	RendererFrontend::UseScene(scene);

	return true;
}

bool update()
{
	Vector3 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), (F32)(Input::ButtonDown(S) - Input::ButtonDown(W)), (F32)(Input::ButtonDown(E) - Input::ButtonDown(Q)) };
	move *= Time::DeltaTime() * 10.0f;
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