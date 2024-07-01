#include "Engine.hpp"

#include "Introspection.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Rendering\Tilemap.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Scene.hpp"
#include "Platform\Input.hpp"
#include "Math\Math.hpp"
#include "Math\Physics.hpp"
#include "Rendering\Sprite.hpp"

import Core;

ResourceRef<Texture> squareCollie;
ResourceRef<Texture> circleCollie;
Scene* scene;
Entity* entity0{};
Entity* player{};
RigidBody2D* playerRB{};
Entity* ground{};
TilemapComponent* tilemap;
U8 id0, id1;

bool Init()
{
	squareCollie = Resources::LoadTexture("textures/Collie.nhtex");
	circleCollie = Resources::LoadTexture("textures/CircleCollie.nhtex");

	scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_ORTHOGRAPHIC);

	//entity0 = scene->AddEntity();
	//tilemap = entity0->AddComponent<TilemapComponent>(100ui16, 100ui16, Vector2{ 10.0f, 10.0f });

	player = scene->AddEntity();
	player->AddComponent<SpriteComponent>(Vector4One, squareCollie);
	playerRB = player->AddComponent<RigidBody2D>(BODY_TYPE_DYNAMIC);
	playerRB->AddCollider(Physics::CreateCircleCollider2D({}, 5.0f));
	//playerRB->SetRotation(45.0f);

	ground = scene->AddEntity();
	ground->AddComponent<SpriteComponent>(Vector4One, squareCollie);
	RigidBody2D* rb = ground->AddComponent<RigidBody2D>(BODY_TYPE_STATIC);
	rb->AddCollider(Physics::CreateCircleCollider2D({}, 5.0f));
	rb->SetPosition({ 0.0f, -25.0f });

	//id0 = tilemap->AddTile(squareCollie);
	//id1 = tilemap->AddTile(Resources::LoadTexture("textures/trichotomy.nhtex"));

	Renderer::LoadScene(scene);

	return true;
}

void Update()
{
	if (Input::OnButtonDown(BUTTON_CODE_LEFT_MOUSE))
	{
		Vector2 pos = Input::MouseToWorld(scene->GetCamera());

		Entity* entity = scene->AddEntity();
		entity->AddComponent<SpriteComponent>(Vector4One, squareCollie);
		RigidBody2D* rb = entity->AddComponent<RigidBody2D>(BODY_TYPE_DYNAMIC);
		rb->AddCollider(Physics::CreateCircleCollider2D({}, 5.0f));
		rb->SetPosition(pos);
	}

	if (Input::OnButtonDown(BUTTON_CODE_RIGHT_MOUSE))
	{
		Vector2 pos = Input::MouseToWorld(scene->GetCamera());

		Entity* entity = scene->AddEntity();
		entity->AddComponent<SpriteComponent>(Vector4One, squareCollie);
		RigidBody2D* rb = entity->AddComponent<RigidBody2D>(BODY_TYPE_STATIC);
		rb->AddCollider(Physics::CreateCircleCollider2D({}, 5.0f));
		rb->SetPosition(pos);
	}

	//if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE)) { tilemap->ChangeTile(tilemap->MouseToTilemap(scene->GetCamera()), id0); }
	////if (Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE)) { tilemap->ChangeTile(tilemap->MouseToTilemap(scene->GetCamera()), id1); }
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
	Engine::Initialize("Tilemap Demo", MakeVersionNumber(0, 1, 0), Engine::DefaultSteamAppId, Init, Update, Shutdown);

	return 0;
}