#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"
#include "Platform\Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Time.hpp"

bool Init()
{
	//SkyboxCreation info{};
	//info.name = "skyboxes/Skybox.sky";
	//info.textureName = "textures/UffiziCube.ktx";
	//info.binaryName = "binaries/Skybox.bin";
	//info.vertexCount = 288;
	//info.indexCount = 72;
	//Skybox* skybox = Resources::CreateSkybox(info);
	//
	//Resources::SaveSkybox(skybox);

	Renderer::LoadScene("scenes/Boombox.scene");

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