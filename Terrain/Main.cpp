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
	Engine::Initialize("Terrain Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}