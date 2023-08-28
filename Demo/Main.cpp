#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Time.hpp"

bool Init()
{
	Model* model = Resources::LoadModel("models/Chess.fbx");

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