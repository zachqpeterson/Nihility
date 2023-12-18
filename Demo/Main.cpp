#include "Engine.hpp"

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

#include <type_traits>

AudioClip* music;
AudioClip* sfx;
F32 volume = 1.0f;
F32 percent = 1.0f;

Entity entity0{};
Entity entity1{};

struct EpicComponent : public Component
{
	void Update() override
	{
		Logger::Debug("Bing Bong");
	}
};

bool Init()
{
	//Components::RegisterComponent<EpicComponent>();
	//Components::CreateComponent<EpicComponent>();

	//String path0 = Resources::UploadModel("ABeautifulGameGLTF/ABeautifulGame.gltf");
	entity0.model = Resources::LoadModel("models/ABeautifulGame.nhmdl");

	//String path = Resources::UploadFont("arial.ttf");
	//String path0 = Resources::UploadSkybox("UffiziCube.ktx");
	//String path1 = Resources::UploadSkybox("CoriolisNight.hdr");

	Scene* scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_PERSPECTIVE, Renderer::GetDefaultRendergraph());
	
	scene->AddEntity(&entity0);
	//scene->AddEntity(&entity1);
	
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
}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}