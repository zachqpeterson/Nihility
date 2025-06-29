#include "Defines.hpp"

#include "Engine.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/UI.hpp"
#include "Rendering/LineRenderer.hpp"
#include "Resources/Resources.hpp"
#include "Platform/Input.hpp"
#include "Math/Random.hpp"
#include "Resources/SpriteComponent.hpp"
#include "Resources/RigidBodyComponent.hpp"
#include "Resources/ColliderComponent.hpp"
#include "Resources/TilemapComponent.hpp"
#include "Resources/TilemapColliderComponent.hpp"
#include "Resources/CharacterComponent.hpp"
#include "Resources/Scene.hpp"

SceneRef scene;
EntityRef player;
ResourceRef<Texture> textureAtlas;
ResourceRef<Texture> groundTexture;
ResourceRef<Texture> whiteTexture;
ComponentRef<Tilemap> mainTilemap;
ComponentRef<Tilemap> backgroundTilemap;
ComponentRef<Tilemap> foregroundTilemap;

void ComponentsInit()
{
	Scene::InitializeFns += RigidBody::Initialize;
	Scene::InitializeFns += Character::Initialize;
	Scene::InitializeFns += Collider::Initialize;
	Scene::InitializeFns += Tilemap::Initialize;
	Scene::InitializeFns += TilemapCollider::Initialize;
	Scene::InitializeFns += Sprite::Initialize;

	Scene::ShutdownFns += Sprite::Shutdown;
	Scene::ShutdownFns += TilemapCollider::Shutdown;
	Scene::ShutdownFns += Tilemap::Shutdown;
	Scene::ShutdownFns += Collider::Shutdown;
	Scene::ShutdownFns += Character::Shutdown;
	Scene::ShutdownFns += RigidBody::Shutdown;
}

bool hover(Element& element)
{
	element.SetColor({ 1.0f, 1.0f, 1.0f, 0.50f });

	return true;
}

bool unhover(Element& element)
{
	element.SetColor({ 1.0f, 1.0f, 1.0f, 0.25f });

	return true;
}

bool click(Element& element)
{
	if (Input::OnButtonDown(ButtonCode::LeftMouse))
	{
		Logger::Debug("Left Click");
	}

	if (Input::OnButtonDown(ButtonCode::RightMouse))
	{
		Logger::Debug("Right Click");
	}

	return true;
}

bool Initialize()
{
	scene = Scene::CreateScene(CameraType::Orthographic);

	whiteTexture = Resources::LoadTexture("textures/white.nht");
	textureAtlas = Resources::LoadTexture("textures/atlas.nht");
	groundTexture = Resources::LoadTexture("textures/missing_texture.nht");

	//ElementInfo info{};
	//info.area = { 0.25f, 0.25f, 0.75f, 0.75f };
	//info.color = { 1.0f, 1.0f, 1.0f, 0.25f };
	//info.texture = groundTexture;
	//
	//ElementRef element = UI::CreateElement(info);
	//
	//element->OnHover += hover;
	//element->OnExit += unhover;
	//element->OnClick += click;
	//
	//ElementInfo textInfo{};
	//textInfo.area = { 0.0f, 0.5f, 1.0f, 1.0f };

	//UI::CreateText({}, "SPHINX OF BLACK QUARTZ,\nJUDGE MY VOW!", 10.0f);
	//UI::CreateText(textInfo, "sphinx of black quartz,\njudge my vow.\n!@#$%^&*()[]{}\\|;:'\",<.>/?`~\n1234567890", 10.0f);

	scene->LoadScene();

	EntityRef ground = scene->CreateEntity({ 0.0f, -10.0f }, { 100.0f, 1.0f });

	ComponentRef<RigidBody> rb = RigidBody::AddTo(ground, BodyType::Static);
	Collider::AddTo(ground, rb);
	Sprite::AddTo(ground, groundTexture);

	player = scene->CreateEntity({ 0.0f, -5.0f });
	Sprite::AddTo(player, groundTexture);
	Character::AddTo(player);

	ResourceRef<AudioClip> clip = Resources::LoadAudio("audio/Electric Zoo.nha");

	U32 effectChain = Audio::CreateEffectChain();
	//Audio::AddReverb(effectChain);
	//Audio::AddEcho(effectChain);
	//Audio::AddLimiter(effectChain);
	//Audio::AddEqualizer(effectChain);
	U32 channel = Audio::CreateChannel(effectChain);

	//Audio::PlayAudioClip(channel, clip);

	EntityRef tilemap = scene->CreateEntity();

	backgroundTilemap = Tilemap::AddTo(tilemap, 100, 100, Vector2::Zero, 0.5f, 1.0f);
	foregroundTilemap = Tilemap::AddTo(tilemap, 100, 100, Vector2::Zero, 1.5f, 0.0f);
	mainTilemap = Tilemap::AddTo(tilemap, 100, 100, Vector2::Zero, 1.0f, 0.5f);

	ComponentRef<RigidBody> tilemapRb = RigidBody::AddTo(tilemap, BodyType::Static);
	TilemapCollider::AddTo(tilemap, tilemapRb, mainTilemap);

	return true;
}

void Shutdown()
{
	scene.Destroy();
}

void Update()
{
	if (Input::ButtonDown(ButtonCode::E))
	{
		Vector2 position;
		position.x = F32(Random::RandomUniform() * 100.0f - 50.0f);
		position.y = F32(Random::RandomUniform() * 60.0f - 30.0f);

		EntityRef id = scene->CreateEntity(position);

		F32 x = Random::RandomRange(0, 2) / 2.0f;
		F32 y = Random::RandomRange(0, 2) / 2.0f;

		Sprite::AddTo(id, textureAtlas, Vector4::One, Vector2{ x, y }, Vector2{ 0.5f, 0.5f });
		ComponentRef<RigidBody> rb = RigidBody::AddTo(id, BodyType::Dynamic);
		Collider::AddTo(id, rb);
	}

	if (Input::ButtonDown(ButtonCode::LeftMouse))
	{
		mainTilemap->SetTile(groundTexture, mainTilemap->ScreenToTilemap(scene->GetCamera(), Input::MousePosition()));
	}

	if (Input::ButtonDown(ButtonCode::RightMouse))
	{
		backgroundTilemap->SetTile(textureAtlas, backgroundTilemap->ScreenToTilemap(scene->GetCamera(), Input::MousePosition()));
	}

	if (Input::ButtonDown(ButtonCode::MiddleMouse))
	{
		foregroundTilemap->SetTile(whiteTexture, foregroundTilemap->ScreenToTilemap(scene->GetCamera(), Input::MousePosition()));
	}
}

int main()
{
	GameInfo game{
		.name = "Nihilty Demo",
		.version = MakeVersionNumber(0, 1, 0),
		.componentsInit = ComponentsInit,
		.initialize = Initialize,
		.shutdown = Shutdown,
		.update = Update,
	};

	Engine::Initialize(game);

	return 0;
}