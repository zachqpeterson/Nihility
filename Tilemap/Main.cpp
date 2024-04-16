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
#include "Core\Time.hpp"
#include "Core\Logger.hpp"

Scene* scene;
Entity* entity{};
TilemapComponent* tilemap;
U8 id0, id1;

bool Init()
{
	scene = Resources::CreateScene("scenes/Chess.nhscn", CAMERA_TYPE_ORTHOGRAPHIC);

	entity = scene->AddEntity();
	tilemap = entity->AddComponent<TilemapComponent>(100ui16, 100ui16, Vector2{ 100.0f, 100.0f });

	id0 = tilemap->AddTile(Resources::LoadTexture("textures/Collie.nhtex"));
	id1 = tilemap->AddTile(Resources::LoadTexture("textures/trichotomy.nhtex"));

	Renderer::LoadScene(scene);

	return true;
}

void Update()
{
	Vector2 eye = scene->GetCamera()->Eye().xy();

	I32 x, y;
	Input::MousePosition(x, y);

	Vector2 worldPos{ x - eye.x, y + eye.y };

	if (worldPos.x >= 0.0f && worldPos.y >= 0.0f)
	{
		if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE)) { tilemap->ChangeTile(tilemap->WorldToTilemap(worldPos), id0); }
		if (Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE)) { tilemap->ChangeTile(tilemap->WorldToTilemap(worldPos), id1); }
		if (Input::ButtonDown(BUTTON_CODE_MIDDLE_MOUSE)) { tilemap->ChangeTile(tilemap->WorldToTilemap(worldPos), U16_MAX); }
	}
}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Tilemap Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}