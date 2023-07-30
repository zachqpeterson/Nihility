#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Time.hpp"

bool Init()
{
	Resources::LoadModel("models/Audi R8/Models/Audi R8.fbx");
	//Renderer::LoadScene("scenes/Boombox.nhscn");

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