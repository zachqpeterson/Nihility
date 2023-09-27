#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Platform\Input.hpp"
#include "Math\Math.hpp"
#include "Core\Time.hpp"

bool Init()
{
	//TODO: Doesn't work because textures are referenced with wide chars, assimp only uses chars
	//String path = Resources::UploadModel("models/Hilichurl/Hilichurl.pmx");

	//String path = Resources::UploadFont("arial.ttf");

	UIElementInfo info{};
	info.area = { -0.5f, -0.5f, 0.5f, 0.5f };
	info.color = { 1.0f, 1.0f, 1.0f, 0.75f };

	info.enabled = true;
	info.ignore = false;

	info.parent = nullptr;
	info.scene = nullptr;

	UI::CreateImage(info, Resources::LoadTexture("textures/Collie.nhtex"), { 0.0f, 0.0f, 1.0f, 1.0f });

	info.area = { -1.0f, -1.0f, -0.5f, -0.5f };
	info.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	UI::CreateText(info, "Hello, World!", 3.0f);

	Model* model = Resources::LoadModel("models/Chess.nhmdl");

	Scene* scene = Resources::CreateScene("scenes/Chess.nhscn");

	scene->AddModel(model);
	//scene->SetSkybox("textures/UffiziCube.ktx");

	Renderer::LoadScene("scenes/Chess.nhscn");

	return true;
}

void Update()
{

}

void Shutdown()
{
	
}

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}