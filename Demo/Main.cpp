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

Scene* scene;
Entity* entity{};
Entity* light{};
UIElement* uiElement;
F32 percent = 0.5f;

bool Init()
{
	//String path = Resources::UploadFont("arial.ttf");
	//String path = Resources::UploadSkybox("Room.hdr");
	//Resources::UploadModel("ABeautifulGameGLTF/ABeautifulGame.gltf");
	//Resources::UploadModel("AnisotropyBarnLamp/AnisotropyBarnLamp.gltf");

	scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_PERSPECTIVE);

	entity = scene->AddEntity();
	light = scene->AddEntity();
	entity->AddComponent<ModelComponent>(Resources::LoadModel("models/AnisotropyBarnLamp.nhmdl"));
	//entity->AddComponent<ModelComponent>(Resources::LoadModel("models/ABeautifulGame.nhmdl"));
	light->AddComponent<MeshComponent>(Resources::LoadMesh("meshes/sphere.nhmsh"), Resources::LoadMaterial("materials/default_material.nhmat"));
	light->transform.SetScale({ 0.001f });
	scene->SetSkybox(Resources::LoadSkybox("textures/Room.nhsky"));

	PostProcessData ppd{};
	ppd.contrast = 1.0f;
	ppd.brightness = 0.0f;
	ppd.saturation = 1.0f;
	ppd.gammaCorrection = 1.0f;

	scene->SetPostProcessing(ppd);

	UIElementInfo info{};
	info.area = { -0.9f, -0.9f, -0.8f, -0.8f };
	info.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	info.scene = scene;

	UIElement* textElement = UI::CreateText(info, "Hello, World!", 5);

	UIElementInfo info1{};
	info.area = { -0.25f, -0.125f, 0.25f, 0.125f };
	info.color = { 0.3f, 0.3f, 0.3f, 1.0f };
	info.scene = scene;

	uiElement = UI::CreateSlider(info, { 1.0f - percent, percent, 0.0f, 1.0f }, SLIDER_TYPE_HORIZONTAL_CENTER, percent);

	Renderer::LoadScene(scene);

	music = Resources::LoadAudio("audio/TheEquable.nhaud");
	sfx = Resources::LoadAudio("audio/Mine.nhaud");

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
		entity->AddComponent<ModelComponent>(Resources::LoadModel("models/AnisotropyBarnLamp.nhmdl"));
	}

	if (Input::ButtonDown(BUTTON_CODE_LEFT))
	{
		percent -= (F32)Time::DeltaTime();
		if (percent < 0.0f) { percent = 0.0f; }

		UI::ChangeSliderPercent(uiElement, percent);
		UI::ChangeSliderColor(uiElement, { 1.0f - percent, percent, 0.0f, 1.0f });
	}

	if (Input::ButtonDown(BUTTON_CODE_RIGHT))
	{
		percent += (F32)Time::DeltaTime();
		if (percent > 1.0f) { percent = 1.0f; }

		UI::ChangeSliderPercent(uiElement, percent);
		UI::ChangeSliderColor(uiElement, { 1.0f - percent, percent, 0.0f, 1.0f });
	}

	Vector3 lightPos;

	lightVal += (F32)Time::DeltaTime();

	lightPos.x = Math::Cos(lightVal) * 0.2f;
	lightPos.y = 1.0f;
	lightPos.z = Math::Sin(lightVal) * 0.2f;

	light->transform.SetPosition(lightPos);

	Quaternion3 rotation(Vector3{ 1.0f, 5.0f, -3.2f } * 5.0f * (F32)(Time::DeltaTime() / 2.0));

	//entity->transform.SetPosition();
	entity->transform.SetRotation(rotation * entity->transform.Rotation());
}

void Shutdown()
{
	music.Destroy();
	sfx.Destroy();
}

int main()
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}