#include "Engine.hpp"

#include "Introspection.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Rendering\Tilemap.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Entity.hpp"
#include "Resources\Scene.hpp"
#include "Platform\Input.hpp"
#include "Platform\Audio.hpp"
#include "Math\Math.hpp"
#include "Math\Physics.hpp"
#include "Core\Time.hpp"
#include "Core\Logger.hpp"
#include "Rendering\Sprite.hpp"

Scene* scene;
Entity* entity0{};
Entity* entity1{};
Entity* entity2{};
Collider2D collider{};
TilemapComponent* tilemap;
U8 id0, id1;

bool Init()
{
	collider.type = COLLIDER_TYPE_AABB;
	collider.aabb.halfWidth = 5.0f;
	collider.aabb.halfHeight = 5.0f;

	scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_ORTHOGRAPHIC);

	//entity0 = scene->AddEntity();
	//tilemap = entity0->AddComponent<TilemapComponent>(100ui16, 100ui16, Vector2{ 100.0f, 100.0f });

	entity1 = scene->AddEntity();
	entity1->AddComponent<SpriteComponent>(Vector4One, Resources::LoadTexture("textures/Collie.nhtex"));
	RigidBody2D* rb0 = entity1->AddComponent<RigidBody2D>();
	rb0->SetCollider(collider);

	//entity2 = scene->AddEntity();
	//entity2->AddComponent<SpriteComponent>(Vector4One, Resources::LoadTexture("textures/trichotomy.nhtex"));
	//RigidBody2D* rb1 = entity2->AddComponent<RigidBody2D>();
	//rb1->SetPosition({0.0f, -20.0f});
	//rb1->SetSimulated(false);
	//rb1->SetCollider(collider);

	ResourceRef<Mesh> mesh = Resources::CreateMesh("mesh");

	//id0 = tilemap->AddTile(Resources::LoadTexture("textures/Collie.nhtex"));
	//id1 = tilemap->AddTile(Resources::LoadTexture("textures/trichotomy.nhtex"));

	Renderer::LoadScene(scene);

	return true;
}

void Update()
{
	if (Input::OnButtonDown(BUTTON_CODE_LEFT_MOUSE))
	{
		Entity* entity = scene->AddEntity();
		entity->AddComponent<SpriteComponent>(Vector4One);
		RigidBody2D* rb = entity->AddComponent<RigidBody2D>();

		Vector2 pos = Input::MouseToWorld(scene->GetCamera());

		rb->SetPosition(pos);
		rb->SetCollider(collider);
	}

	if (Input::OnButtonDown(BUTTON_CODE_RIGHT_MOUSE))
	{
		Entity* entity = scene->AddEntity();
		entity->AddComponent<SpriteComponent>(Vector4One);
		RigidBody2D* rb = entity->AddComponent<RigidBody2D>();

		Vector2 pos = Input::MouseToWorld(scene->GetCamera());

		rb->SetPosition(pos);
		rb->SetCollider(collider);
		rb->SetSimulated(false);
	}

	//Vector2 worldPos = Input::MousePosition() + eye.xy();
	//
	//if (worldPos.x >= 0.0f && worldPos.y >= 0.0f)
	//{
	//	if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE)) { tilemap->ChangeTile(tilemap->WorldToTilemap(worldPos), id0); }
	//	if (Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE)) { tilemap->ChangeTile(tilemap->WorldToTilemap(worldPos), id1); }
	//	if (Input::ButtonDown(BUTTON_CODE_MIDDLE_MOUSE)) { tilemap->ChangeTile(tilemap->WorldToTilemap(worldPos), U8_MAX); }
	//}
}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Tilemap Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}