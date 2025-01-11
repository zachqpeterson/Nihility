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
	GameInfo info{};
	info.GameInit = Init;
	info.GameUpdate = Update;
	info.GameShutdown = Shutdown;
	info.gameName = "Terrain Demo";
	info.gameVersion = MakeVersionNumber(0, 1, 0);
	info.steamAppId = 0;
	info.discordAppId = 0;

	Engine::Initialize(info);

	return 0;
}