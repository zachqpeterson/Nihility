#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Platform\Input.hpp"
#include "Platform\Audio.hpp"
#include "Math\Math.hpp"
#include "Core\Time.hpp"
#include "Core\Logger.hpp"

AudioClip* music;
AudioClip* sfx;
F32 volume = 1.0f;

bool Init()
{
	//TODO: Doesn't work because textures are referenced with wide chars, assimp only uses chars
	//String path = Resources::UploadModel("models/Hilichurl/Hilichurl.pmx");

	//String path = Resources::UploadFont("arial.ttf");
	//String path = Resources::UploadSkybox("UffiziCube.ktx");

	UIElementInfo info{};
	info.area = { -0.5f, -0.5f, 0.5f, 0.5f };
	info.color = { 1.0f, 1.0f, 1.0f, 0.75f };

	info.enabled = true;
	info.ignore = false;

	info.parent = nullptr;
	info.scene = nullptr;
	//info.OnClick = Click;
	//info.OnDrag = Drag;
	//info.OnRelease = Release;
	//info.OnHover = Hover;
	//info.OnMove = Moved;
	//info.OnExit = Exit;
	//info.OnScroll = Scroll;

	UI::CreatePanel(info, 1.0f, {0.5f, 0.5f, 0.5f, 0.75f});

	info.area = { -1.0f, -1.0f, -0.5f, -0.5f };
	info.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	UI::CreateText(info, "Hgj,|_\nHello", 10.0f);

	Scene* scene = Resources::CreateScene("scenes/Chess.nhscn");

	Model* model = Resources::LoadModel("models/Chess.nhmdl");
	scene->AddModel(model);

	Skybox* skybox = Resources::LoadSkybox("textures/UffiziCube.nhskb");
	scene->SetSkybox(skybox);

	Renderer::LoadScene("scenes/Chess.nhscn");

	music = Resources::LoadAudio("audio/TPOM.nhaud");
	sfx = Resources::LoadAudio("audio/Mine.nhaud");
	Audio::PlayMusic(music);

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
}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}