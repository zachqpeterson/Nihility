#include "Defines.hpp"

#include "Engine.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/UI.hpp"
#include "Rendering/LineRenderer.hpp"
#include "Resources/Resources.hpp"
#include "Platform/Input.hpp"
#include "Math/Random.hpp"
#include "Resources/SpriteComponent.hpp"
#include "Resources/ProjectileComponent.hpp"
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
	Scene::InitializeFns += Character::Initialize;
	Scene::InitializeFns += Projectile::Initialize;
	Scene::InitializeFns += Collider::Initialize;
	Scene::InitializeFns += Tilemap::Initialize;
	Scene::InitializeFns += TilemapCollider::Initialize;
	Scene::InitializeFns += Sprite::Initialize;

	Scene::ShutdownFns += Sprite::Shutdown;
	Scene::ShutdownFns += TilemapCollider::Shutdown;
	Scene::ShutdownFns += Tilemap::Shutdown;
	Scene::ShutdownFns += Collider::Shutdown;
	Scene::ShutdownFns += Projectile::Shutdown;
	Scene::ShutdownFns += Character::Shutdown;
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

bool ProjectileHit(const EntityRef& ref)
{
	//TODO: Spawn particles

	Sprite::RemoveFrom(ref);
	Projectile::RemoveFrom(ref);
	scene->DestroyEntity(ref);
	return false;
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

	EntityRef ground = scene->CreateEntity({ 0.0f, -5.0f }, { 100.0f, 1.0f });
	EntityRef ground1 = scene->CreateEntity({ 5.0f, -4.0f }, { 1.0f, 6.0f });
	EntityRef ground2 = scene->CreateEntity({ 0.0f, 8.0f }, { 100.0f, 1.0f });

	Collider::AddTo(ground);
	Sprite::AddTo(ground, groundTexture);

	Collider::AddTo(ground1);
	Sprite::AddTo(ground1, groundTexture);

	Collider::AddTo(ground2);
	Sprite::AddTo(ground2, groundTexture);

	player = scene->CreateEntity({ 0.0f, 0.0f });
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

	TilemapCollider::AddTo(tilemap, mainTilemap);

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
		EntityRef id = scene->CreateEntity(player->position, 0.5f);

		ComponentRef<Sprite> s = Sprite::AddTo(id, groundTexture);
		ComponentRef<Projectile> p = Projectile::AddTo(id, (scene->ScreenToWorld(Input::MousePosition()) - player->position).Normalized() * 20.0f, 0.0f, 0.0f);
		if (s && p) { p->OnHit += ProjectileHit; }
	}

	if (Input::ButtonDown(ButtonCode::LeftMouse))
	{
		EntityRef id = scene->CreateEntity(scene->ScreenToWorld(Input::MousePosition()));

		ComponentRef<Sprite> s = Sprite::AddTo(id, groundTexture);
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