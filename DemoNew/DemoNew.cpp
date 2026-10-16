#include "Nihility.hpp"

#include "Resources/Scene.hpp"
#include "Rendering/UI.hpp"
#include "Rendering/Renderer.hpp"
#include "Platform/Input.hpp"

class LevelOneScene : public Scene
{
public:
	void OnStart() override
	{
		LoadTilemap("levels/Test.tilemap");
		LoadEntities("levels/Test.entities");

		Entity light = CreateSceneEntity({ 500.0f, 550.0f });

		PointLight& pl0 = light.AddComponent<PointLight>();
		pl0.color = { 1.0f, 1.0f, 1.0f };
		pl0.radius = 100.0f;
		pl0.intensity = 1.0f;

		Entity player = SpawnPlayer({ 0.0f, 100.0f });
		Renderer::SetActiveCamera(SetupCamera(player));
	}

	void OnUpdate() override
	{

	}
};

class MainMenuScene : public Scene
{
public:
	void OnStart() override
	{
		SetupCamera();

		Button startButton = UI::CreateButton({ { 0_px, 0_px }, { 100_px, 30_px }, { 0.5f, 0.2f } }, "Play");

		AddToScene(startButton.entity);

		startButton.interactable.onClickName = "play_level_one";

		UI::RegisterAction(startButton.interactable.onClickName, []() {
			SceneManager::ChangeScene(std::make_shared<LevelOneScene>());
		});

		Renderer::SetActiveCamera(SetupCamera());
	}
};

void InitGame()
{
	SceneManager::ChangeScene(std::make_shared<MainMenuScene>());

	Input::BindAxis("Horizontal", ButtonCode::A, -1.0f);
	Input::BindAxis("Horizontal", ButtonCode::D, 1.0f);
	Input::BindAction("Jump", ButtonCode::Space);
}

void ShutdownGame()
{

}

void RunGame()
{

}

int main()
{
	GameInfo info{};
	info.applicationName = L"DemoNew";
	info.versionNumber = MakeVersionNumber(0, 1, 0);
	info.InitializeGame = InitGame;
	info.ShutdownGame = ShutdownGame;
	info.RunGame = RunGame;

	Nihility::Initialize(info);
}