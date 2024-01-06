#include "Engine.hpp"

#include "Introspection.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Entity.hpp"
#include "Resources\Scene.hpp"
#include "Platform\Input.hpp"
#include "Platform\Audio.hpp"
#include "Math\Math.hpp"
#include "Core\Time.hpp"
#include "Core\Logger.hpp"

AudioClip* music;
AudioClip* sfx;
F32 volume = 1.0f;
F32 percent = 1.0f;

Entity* entity{};

bool Init()
{
	//String path = Resources::UploadFont("arial.ttf");
	//String path0 = Resources::UploadSkybox("UffiziCube.ktx");
	//String path = Resources::UploadSkybox("Room.hdr");
	//Resources::UploadModel("ABeautifulGameGLTF/ABeautifulGame.gltf");
	//Resources::UploadModel("AnisotropyBarnLamp/glTF/AnisotropyBarnLamp.gltf");

	Scene* scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_PERSPECTIVE, Renderer::GetDefaultRendergraph());
	
	entity = scene->AddEntity();
	//entity->AddComponent<ModelComponent>(Resources::LoadModel("models/AnisotropyBarnLamp.nhmdl"));
	entity->AddComponent<ModelComponent>(Resources::LoadModel("models/ABeautifulGame.nhmdl"));
	scene->SetSkybox(Resources::LoadSkybox("textures/Room.nhsky"));

	Renderer::LoadScene(scene);
	
	music = Resources::LoadAudio("audio/TPOM.nhaud");
	sfx = Resources::LoadAudio("audio/Mine.nhaud");
	//Audio::PlayMusic(music);

	return true;
}

void Update()
{
	if (Input::ButtonDown(BUTTON_CODE_SPACE))
	{
		Audio::PlaySfx(sfx);
	}
	
	if (Input::OnButtonDown(BUTTON_CODE_M))
	{
		Audio::PlayMusic(music);
	}
	
	if (Input::MouseWheelDelta())
	{
		volume += 0.1f * Input::MouseWheelDelta();
		Audio::ChangeMusicVolume(volume);
	}

	if (Input::OnButtonDown(BUTTON_CODE_V))
	{
		Settings::SetVSync(!Settings::VSync());
	}

	if (Input::ButtonDown(BUTTON_CODE_LEFT))
	{
		entity->transform.Translate(Vector3Left * Time::DeltaTime() * 10.0f);
	}

	if (Input::ButtonDown(BUTTON_CODE_RIGHT))
	{
		entity->transform.Translate(Vector3Right * Time::DeltaTime() * 10.0f);
	}

	if (Input::ButtonDown(BUTTON_CODE_UP))
	{
		entity->transform.Translate(Vector3Up * Time::DeltaTime() * 10.0f);
	}

	if (Input::ButtonDown(BUTTON_CODE_DOWN))
	{
		entity->transform.Translate(Vector3Down * Time::DeltaTime() * 10.0f);
	}
}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}