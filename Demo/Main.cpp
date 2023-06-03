#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Time.hpp"

Scene* scene;

bool Init()
{
	scene = Resources::LoadScene("scenes/Boombox.scene");
	scene->camera.SetPerspective(0.1f, 4000.0f, 60.0f, (F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());

	return true;
}

void Update()
{
	scene->Update();
}

void Shutdown()
{

}

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}