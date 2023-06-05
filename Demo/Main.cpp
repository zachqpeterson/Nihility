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

	//Resources::SaveScene(scene);

	return true;
}

void Update()
{
	scene->Update();
}

void Shutdown()
{
	Resources::SaveScene(scene);
}

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}