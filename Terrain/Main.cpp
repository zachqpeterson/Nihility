#include "Engine.hpp"

bool Init()
{
	return true;
}

void Update()
{

}

void Shutdown()
{

}

int main()
{
	Engine::Initialize("Terrain Demo", MakeVersionNumber(0, 1, 0), Engine::DefaultSteamAppId, Init, Update, Shutdown);

	return 0;
}