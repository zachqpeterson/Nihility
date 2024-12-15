#include "Engine.hpp"

#include "Introspection.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Rendering\Tilemap.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Scene.hpp"
#include "Math\Math.hpp"
#include "Math\Physics.hpp"
#include "Rendering\Sprite.hpp"

import Core;
import Input;

ResourceRef<Texture> squareCollie;
ResourceRef<Texture> circleCollie;
Scene* scene;
Entity* entity0{};
Entity* player{};
ComponentRef<RigidBody2D> playerRB{};
Entity* ground{};
ComponentRef<TilemapComponent> tilemap;
U8 id0, id1;

bool Init()
{
	squareCollie = Resources::LoadTexture("textures/Collie.nhtex");
	circleCollie = Resources::LoadTexture("textures/CircleCollie.nhtex");

	scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_ORTHOGRAPHIC);

	entity0 = scene->CreateEntity();
	//tilemap = entity0->CreateComponent<TilemapComponent>(100ui16, 100ui16, Vector2{ 10.0f, 10.0f });

	player = scene->CreateEntity();
	player->CreateComponent<SpriteComponent>(Vector4One, squareCollie);

	RigidBody2DDef playerDef{};
	playerDef.type = BODY_TYPE_DYNAMIC;
	playerDef.position.y = 25.0f;
	RigidBody2DDef groundDef{};
	groundDef.position.y = -25.0f;
	ShapeDef shapeDef{};

	playerRB = player->CreateComponent<RigidBody2D>(playerDef);
	playerRB->AddCollider(shapeDef, Physics::CreateBox(5.0f, 5.0f));
	//playerRB->SetPosition({ 0.0f, 25.0f });
	//playerRB->SetRotation(45.0f);

	ground = scene->CreateEntity();
	ground->CreateComponent<SpriteComponent>(Vector4One, squareCollie);
	ComponentRef<RigidBody2D> rb = ground->CreateComponent<RigidBody2D>(groundDef);
	rb->AddCollider(shapeDef, Physics::CreateBox(5.0f, 5.0f));
	//rb->SetPosition({ 0.0f, -25.0f });

	//id0 = tilemap->AddTile(squareCollie);
	//id1 = tilemap->AddTile(Resources::LoadTexture("textures/trichotomy.nhtex"));

	Renderer::LoadScene(scene);

	return true;
}

void Update()
{
	ShapeDef shapeDef{};

	//if (Input::OnButtonDown(BUTTON_CODE_LEFT_MOUSE))
	//{
	//	Vector2 pos = Input::MouseToWorld(scene->GetCamera());
	//
	//	Entity* entity = scene->CreateEntity();
	//	entity->CreateComponent<SpriteComponent>(Vector4One, squareCollie);
	//	ComponentRef<RigidBody2D> rb = entity->CreateComponent<RigidBody2D>(BODY_TYPE_DYNAMIC);
	//	rb->AddCollider(shapeDef, Physics::CreateBox(5.0f, 5.0f));
	//	rb->SetPosition(pos);
	//}
	//
	//if (Input::OnButtonDown(BUTTON_CODE_RIGHT_MOUSE))
	//{
	//	Vector2 pos = Input::MouseToWorld(scene->GetCamera());
	//
	//	Entity* entity = scene->CreateEntity();
	//	entity->CreateComponent<SpriteComponent>(Vector4One, squareCollie);
	//	ComponentRef<RigidBody2D> rb = entity->CreateComponent<RigidBody2D>(BODY_TYPE_STATIC);
	//	rb->AddCollider(shapeDef, Physics::CreateBox(5.0f, 5.0f));
	//	rb->SetPosition(pos);
	//}

	//if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE)) { tilemap->ChangeTile(tilemap->MouseToTilemap(scene->GetCamera()), id0); }
	//if (Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE)) { tilemap->ChangeTile(tilemap->MouseToTilemap(scene->GetCamera()), id1); }
	//if (Input::ButtonDown(BUTTON_CODE_MIDDLE_MOUSE)) { tilemap->ChangeTile(tilemap->MouseToTilemap(scene->GetCamera()), U8_MAX); }

	F32 speed = 1.0f;
	F32 jumpForce = 20.0f;

	//if (Input::ButtonDown(BUTTON_CODE_A)) { playerRB->AddVelocity(Vector2Left * speed); }
	//if (Input::ButtonDown(BUTTON_CODE_D)) { playerRB->AddVelocity(Vector2Right * speed); }
	//if (Input::OnButtonDown(BUTTON_CODE_SPACE)) { playerRB->AddVelocity(Vector2Up * jumpForce); }
}

void Shutdown()
{

}

int main()
{
	GameInfo info{};
	info.GameInit = Init;
	info.GameUpdate = Update;
	info.GameShutdown = Shutdown;
	info.gameName = "Tilemap Demo";
	info.gameVersion = 0;
	info.steamAppId = 0;
	info.discordAppId = 0;

	Engine::Initialize(info);

	return 0;
}