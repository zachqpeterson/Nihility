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

ResourceRef<AudioClip> music;
ResourceRef<AudioClip> sfx;
F32 volume = 1.0f;
F32 percent = 1.0f;

Entity* entity{};
Entity* light{};

bool Init()
{
	//String path = Resources::UploadFont("arial.ttf");
	//String path = Resources::UploadSkybox("Room.hdr");
	//Resources::UploadModel("ABeautifulGameGLTF/ABeautifulGame.gltf");
	//Resources::UploadModel("AnisotropyBarnLamp/AnisotropyBarnLamp.gltf");

	Scene* scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_PERSPECTIVE);
	
	entity = scene->AddEntity();
	light = scene->AddEntity();
	entity->AddComponent<ModelComponent>(Resources::LoadModel("models/AnisotropyBarnLamp.nhmdl"));
	//entity->AddComponent<ModelComponent>(Resources::LoadModel("models/ABeautifulGame.nhmdl"));
	light->AddComponent<MeshComponent>(Resources::LoadMesh("meshes/sphere.nhmsh"), Resources::LoadMaterial("materials/default.nhmat"));
	light->transform.SetScale({ 0.001f });
	scene->SetSkybox(Resources::LoadSkybox("textures/Room.nhsky"));

	UIElementInfo info{};
	info.area = { 0.1f, 0.1f, 0.2f, 0.2f };
	info.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	info.scene = scene;

	//UIElement* element = UI::CreateText(info, "Hello, World!", 10);

	Renderer::LoadScene(scene);
	
	music = Resources::LoadAudio("audio/TheEquable.nhaud");
	sfx = Resources::LoadAudio("audio/Mine.nhaud");
	//Audio::PlayMusic(music);

	return true;
}

F32 lightVal = 0;

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
		entity->transform.Translate(Vector3Left * (F32)Time::DeltaTime());
	}

	if (Input::ButtonDown(BUTTON_CODE_RIGHT))
	{
		entity->transform.Translate(Vector3Right * (F32)Time::DeltaTime());
	}

	if (Input::ButtonDown(BUTTON_CODE_UP))
	{
		entity->transform.Translate(Vector3Forward * (F32)Time::DeltaTime());
	}

	if (Input::ButtonDown(BUTTON_CODE_DOWN))
	{
		entity->transform.Translate(Vector3Back * (F32)Time::DeltaTime());
	}

	if (Input::OnButtonDown(BUTTON_CODE_P))
	{
		Input::SetMousePosition(0, 0);
	}

	if (Input::OnButtonDown(BUTTON_CODE_O))
	{
		Input::LockCursor(!Settings::CursorLocked());
	}

	if (Input::OnButtonDown(BUTTON_CODE_I))
	{
		Input::ConstrainCursor(!Settings::CursorConstrained());
	}

	if (Input::OnButtonDown(BUTTON_CODE_U))
	{
		Input::ShowCursor(!Settings::CursorShowing());
	}

	if (Input::OnButtonDown(BUTTON_CODE_H))
	{
		//TODO: Fix runtime model loading
		entity->AddComponent<ModelComponent>(Resources::LoadModel("models/AnisotropyBarnLamp.nhmdl"));
	}

	Vector3 lightPos;

	lightVal += (F32)Time::DeltaTime();

	lightPos.x = Math::Cos(lightVal) * 0.2f;
	lightPos.y = 1.0f;
	lightPos.z = Math::Sin(lightVal) * 0.2f;

	light->transform.SetPosition(lightPos);
}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}