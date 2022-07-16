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

	Mesh* backgroundMesh = Resources::CreateMesh(backgroundConfig);
	Mesh* mesh0 = Resources::CreateMesh(config0);

	UIElementConfig config{};
	config.area = { 0.25f, 0.25f, 0.75f, 0.75f };
	config.color = { 1.0f, 0.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.name = "Panel0";
	config.scene = scene;

	UI::GeneratePanel(config);

	config.name = "Panel1";
	config.parentName = "Panel0";
	config.color = { 0.0f, 1.0f, 0.0f, 1.0f };
	UI::GenerateBorderedPanel(config);

	config.name = "Image0";
	config.parentName = "Panel1";
	config.color = Vector4::ONE;
	UI::GenerateImage(config, Resources::DefaultTexture());

	scene->DrawMesh(backgroundMesh);
	scene->DrawMesh(mesh0);

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