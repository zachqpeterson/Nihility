#include "Engine.hpp"

bool init()
{
	return true;
}

bool update()
{
	return true;
}

void shutdown()
{

}

int main()
{
	Engine::Initialize(L"Nihility Demo", init, update, shutdown);

	return 0;
}