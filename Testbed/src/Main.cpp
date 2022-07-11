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

Scene* scene;

bool init()
{
	scene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	scene->Create(CAMERA_TYPE_ORTHOGRAPHIC);

	MeshConfig config0;
	config0.name = "Mesh0";
	config0.MaterialName = "Tile.mat";

	//FRONT TILE
	config0.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.33333333333f, 0.625f }, });
	config0.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.5f, 0.625f }, });
	config0.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.5f, 0.5f }, });
	config0.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.33333333333f, 0.5f }, });

	config0.indices.Push(0);
	config0.indices.Push(1);
	config0.indices.Push(2);
	config0.indices.Push(2);
	config0.indices.Push(3);
	config0.indices.Push(0);

	MeshConfig config1;
	config1.name = "Mesh1";
	config1.MaterialName = "Tile.mat";
	//BACK TILE
	config1.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.375f }, });
	config1.vertices.Push(Vertex{ { 1.5f, -0.5f, 0.0f}, { 0.33333333333f, 0.375f }, });
	config1.vertices.Push(Vertex{ { 1.5f,  0.5f, 0.0f}, { 0.33333333333f, 0.25f }, });
	config1.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.25f }, });

	config1.indices.Push(0);
	config1.indices.Push(1);
	config1.indices.Push(2);
	config1.indices.Push(2);
	config1.indices.Push(3);
	config1.indices.Push(0);

	Mesh* mesh0 = Resources::CreateMesh(config0);
	Mesh* mesh1 = Resources::CreateMesh(config1);
	scene->DrawMesh(mesh0, Matrix4::IDENTITY);
	scene->DrawMesh(mesh1, Matrix4::IDENTITY);

	RendererFrontend::UseScene(scene);

	return true;
}

bool update()
{
	Vector3 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), (F32)(Input::ButtonDown(S) - Input::ButtonDown(W)), (F32)(Input::ButtonDown(E) - Input::ButtonDown(Q)) };
	move *= Time::DeltaTime() * 200.0f;
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